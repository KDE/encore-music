// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "player.h"
#include "tagreader.h"

#include <QFuture>
#include <QJSEngine>
#include <QRandomGenerator>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

// If a track is only 3s in, going back means going to the previous track
constexpr qint64 RestartThresholdMs = 3000;

Player *Player::instance()
{
    static Player *player = new Player;
    return player;
}

Player *Player::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    Player *player = instance();
    QJSEngine::setObjectOwnership(player, QJSEngine::CppOwnership);
    return player;
}

Player::Player(QObject *parent)
    : QObject(parent)
    , m_queue(new PlayQueue(this))
{
    m_mediaPlayer.setAudioOutput(&m_audioOutput);

    connect(&m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &Player::playbackStateChanged);
    connect(&m_mediaPlayer, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(&m_mediaPlayer, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
    connect(&m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &Player::handleMediaStatus);
    connect(&m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &Player::handleError);
    connect(&m_audioOutput, &QAudioOutput::volumeChanged, this, &Player::volumeChanged);

    connect(m_queue, &PlayQueue::currentTrackChanged, this, [this] {
        loadCurrentTrack();
        Q_EMIT currentTrackChanged();
        Q_EMIT navigationChanged();
    });
    connect(m_queue, &PlayQueue::currentIndexChanged, this, &Player::navigationChanged);
    connect(m_queue, &PlayQueue::countChanged, this, &Player::navigationChanged);
}

PlayQueue *Player::queue() const
{
    return m_queue;
}

Track Player::currentTrack() const
{
    return m_queue->currentTrack();
}

bool Player::hasTrack() const
{
    return m_queue->currentIndex() >= 0;
}

Player::PlaybackState Player::playbackState() const
{
    switch (m_mediaPlayer.playbackState()) {
    case QMediaPlayer::PlayingState:
        return Playing;
    case QMediaPlayer::PausedState:
        return Paused;
    case QMediaPlayer::StoppedState:
        break;
    }
    return Stopped;
}

bool Player::isPlaying() const
{
    return playbackState() == Playing;
}

qint64 Player::position() const
{
    return m_mediaPlayer.position();
}

qint64 Player::duration() const
{
    const qint64 duration = m_mediaPlayer.duration();
    return duration > 0 ? duration : currentTrack().duration;
}

qreal Player::volume() const
{
    return m_audioOutput.volume();
}

void Player::setVolume(qreal volume)
{
    m_audioOutput.setVolume(std::clamp(volume, 0.0, 1.0));
}

bool Player::shuffle() const
{
    return m_queue->isShuffled();
}

void Player::setShuffle(bool shuffle)
{
    if (m_queue->isShuffled() == shuffle) {
        return;
    }
    m_queue->setShuffled(shuffle);
    Q_EMIT shuffleChanged();
}

Player::RepeatMode Player::repeatMode() const
{
    return m_repeatMode;
}

void Player::setRepeatMode(RepeatMode mode)
{
    if (m_repeatMode == mode) {
        return;
    }
    m_repeatMode = mode;
    Q_EMIT repeatModeChanged();
    Q_EMIT navigationChanged();
}

bool Player::canGoNext() const
{
    const int index = m_queue->currentIndex();
    return index >= 0 && (index + 1 < m_queue->count() || m_repeatMode == RepeatAll);
}

bool Player::canGoPrevious() const
{
    return hasTrack();
}

bool Player::isLoading() const
{
    return m_pendingLoads > 0;
}

QString Player::formatTime(qint64 milliseconds)
{
    const qint64 totalSeconds = std::max<qint64>(0, milliseconds / 1000);
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    if (hours > 0) {
        return u"%1:%2:%3"_s.arg(hours).arg(minutes, 2, 10, u'0').arg(seconds, 2, 10, u'0');
    }
    return u"%1:%2"_s.arg(minutes).arg(seconds, 2, 10, u'0');
}

void Player::play()
{
    if (!hasTrack()) {
        return;
    }
    m_playWhenLoaded = true;
    m_mediaPlayer.play();
}

void Player::pause()
{
    m_playWhenLoaded = false;
    m_mediaPlayer.pause();
}

void Player::togglePlayPause()
{
    if (isPlaying()) {
        pause();
    } else {
        play();
    }
}

void Player::stop()
{
    m_playWhenLoaded = false;
    m_mediaPlayer.stop();
}

void Player::next()
{
    const int index = m_queue->currentIndex();
    if (index + 1 < m_queue->count()) {
        m_queue->setCurrentIndex(index + 1);
    } else if (m_repeatMode == RepeatAll && !m_queue->isEmpty()) {
        m_queue->setCurrentIndex(0);
    }
}

void Player::previous()
{
    const int index = m_queue->currentIndex();
    if (index > 0 && position() < RestartThresholdMs) {
        m_queue->setCurrentIndex(index - 1);
    } else {
        seek(0);
    }
}

void Player::seek(qint64 position)
{
    position = std::clamp<qint64>(position, 0, std::max<qint64>(0, duration()));
    m_mediaPlayer.setPosition(position);
    Q_EMIT seeked(position);
}

void Player::playIndex(int index)
{
    if (index < 0 || index >= m_queue->count()) {
        return;
    }
    m_playWhenLoaded = true;
    if (index == m_queue->currentIndex()) {
        seek(0);
        m_mediaPlayer.play();
    } else {
        m_queue->setCurrentIndex(index);
    }
}

void Player::openUrls(const QList<QUrl> &urls)
{
    load(urls, LoadAction::Replace);
}

void Player::enqueue(const QList<QUrl> &urls)
{
    load(urls, LoadAction::Append);
}

void Player::playNext(const QList<QUrl> &urls)
{
    load(urls, LoadAction::InsertNext);
}

void Player::playTracks(const QList<Track> &tracks, int startIndex, bool shuffle)
{
    if (tracks.isEmpty()) {
        return;
    }

    m_playWhenLoaded = true;
    if (shuffle) {
        startIndex = QRandomGenerator::global()->bounded(int(tracks.size()));
        setShuffle(true);
    }
    // With shuffle on, the queue puts the start track first and shuffles the rest
    m_queue->setTracks(tracks, startIndex);
}

void Player::load(const QList<QUrl> &urls, LoadAction action)
{
    if (urls.isEmpty()) {
        return;
    }

    if (m_pendingLoads++ == 0) {
        Q_EMIT loadingChanged();
    }

    // Reading tags can take a while for a big folder, so it happens off the UI thread
    QtConcurrent::run([urls] {
        TagReader reader;
        QList<Track> tracks;
        const QStringList files = reader.findAudioFiles(urls);
        tracks.reserve(files.size());
        for (const QString &file : files) {
            tracks.append(reader.read(file));
        }
        return tracks;
    }).then(this, [this, action](const QList<Track> &tracks) {
        if (--m_pendingLoads == 0) {
            Q_EMIT loadingChanged();
        }

        if (tracks.isEmpty()) {
            Q_EMIT errorOccurred(u"No audio files found"_s);
            return;
        }

        switch (action) {
        case LoadAction::Replace:
            m_playWhenLoaded = true;
            m_queue->setTracks(tracks, 0);
            break;
        case LoadAction::Append:
            m_queue->append(tracks);
            break;
        case LoadAction::InsertNext:
            m_queue->insertNext(tracks);
            break;
        }

        // Adding to an empty queue selects the first track, ready to play
        if (!hasTrack()) {
            m_queue->setCurrentIndex(0);
        }
    });
}

void Player::loadCurrentTrack()
{
    const Track track = m_queue->currentTrack();
    if (m_mediaPlayer.source() == track.url) {
        // If already playing, just reset track position
        m_mediaPlayer.setPosition(0);
    } else {
        m_mediaPlayer.setSource(track.url);
    }

    if (track.isValid() && m_playWhenLoaded) {
        m_mediaPlayer.play();
    }
    Q_EMIT durationChanged();
}

void Player::handleMediaStatus(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::BufferedMedia:
        m_consecutiveErrors = 0;
        break;
    case QMediaPlayer::EndOfMedia:
        // End of track behavior
        if (m_repeatMode == RepeatOne) {
            seek(0);
            m_mediaPlayer.play();
        } else if (!advance()) {
            // Stay on the last track, ready to play again from the start
            m_playWhenLoaded = false;
            m_mediaPlayer.stop();
        }
        break;
    default:
        break;
    }
}

void Player::handleError(QMediaPlayer::Error error, const QString &message)
{
    if (error == QMediaPlayer::NoError) {
        return;
    }

    qWarning() << "Could not play" << m_mediaPlayer.source() << message;
    Q_EMIT errorOccurred(message);

    // Skip the broken track, but give up once every track has failed
    if (++m_consecutiveErrors < m_queue->count()) {
        advance();
    } else {
        m_playWhenLoaded = false;
    }
}

bool Player::advance()
{
    const int index = m_queue->currentIndex();
    if (index + 1 < m_queue->count()) {
        m_queue->setCurrentIndex(index + 1);
        return true;
    }
    if (m_repeatMode == RepeatAll && !m_queue->isEmpty()) {
        if (m_queue->count() == 1) {
            seek(0);
            m_mediaPlayer.play();
        } else {
            m_queue->setCurrentIndex(0);
        }
        return true;
    }
    return false;
}
