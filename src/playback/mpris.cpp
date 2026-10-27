// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "mpris.h"
#include "player.h"

#include <KAboutData>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>

using namespace Qt::StringLiterals;

const QString ObjectPath = u"/org/mpris/MediaPlayer2"_s;
const QString PlayerInterface = u"org.mpris.MediaPlayer2.Player"_s;

static qlonglong toMicroseconds(qint64 milliseconds)
{
    return milliseconds * 1000;
}

Mpris::Mpris(Player *player, QObject *parent)
    : QObject(parent)
{
    new MediaPlayer2Adaptor(this);
    new MediaPlayer2PlayerAdaptor(player, this);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(ObjectPath, this, QDBusConnection::ExportAdaptors);
    bus.registerService(u"org.mpris.MediaPlayer2.encoremusic"_s);
}

MediaPlayer2Adaptor::MediaPlayer2Adaptor(Mpris *mpris)
    : QDBusAbstractAdaptor(mpris)
    , m_mpris(mpris)
{
}

bool MediaPlayer2Adaptor::canQuit() const
{
    return true;
}

bool MediaPlayer2Adaptor::canRaise() const
{
    return true;
}

bool MediaPlayer2Adaptor::hasTrackList() const
{
    return false;
}

QString MediaPlayer2Adaptor::identity() const
{
    return KAboutData::applicationData().displayName();
}

QString MediaPlayer2Adaptor::desktopEntry() const
{
    return KAboutData::applicationData().desktopFileName();
}

QStringList MediaPlayer2Adaptor::supportedUriSchemes() const
{
    return {u"file"_s};
}

QStringList MediaPlayer2Adaptor::supportedMimeTypes() const
{
    return {
        u"audio/aac"_s,
        u"audio/flac"_s,
        u"audio/mp4"_s,
        u"audio/mpeg"_s,
        u"audio/ogg"_s,
        u"audio/opus"_s,
        u"audio/wav"_s,
        u"audio/x-flac"_s,
        u"audio/x-m4a"_s,
        u"audio/x-vorbis+ogg"_s,
        u"audio/x-wav"_s,
    };
}

void MediaPlayer2Adaptor::Raise()
{
    Q_EMIT m_mpris->raiseRequested();
}

void MediaPlayer2Adaptor::Quit()
{
    QCoreApplication::quit();
}

MediaPlayer2PlayerAdaptor::MediaPlayer2PlayerAdaptor(Player *player, Mpris *mpris)
    : QDBusAbstractAdaptor(mpris)
    , m_player(player)
{
    connect(player, &Player::playbackStateChanged, this, [this] {
        notifyChanged({{u"PlaybackStatus"_s, playbackStatus()}, {u"CanPlay"_s, canPlay()}, {u"CanPause"_s, canPause()}});
    });
    connect(player, &Player::currentTrackChanged, this, [this] {
        notifyChanged({{u"Metadata"_s, metadata()}, {u"CanSeek"_s, canSeek()}, {u"CanPlay"_s, canPlay()}, {u"CanPause"_s, canPause()}});
    });
    // The real duration replaces the one from the tags once the media has loaded
    connect(player, &Player::durationChanged, this, [this] {
        notifyChanged({{u"Metadata"_s, metadata()}});
    });
    connect(player, &Player::navigationChanged, this, [this] {
        notifyChanged({{u"CanGoNext"_s, canGoNext()}, {u"CanGoPrevious"_s, canGoPrevious()}});
    });
    connect(player, &Player::volumeChanged, this, [this] {
        notifyChanged({{u"Volume"_s, volume()}});
    });
    connect(player, &Player::shuffleChanged, this, [this] {
        notifyChanged({{u"Shuffle"_s, shuffle()}});
    });
    connect(player, &Player::repeatModeChanged, this, [this] {
        notifyChanged({{u"LoopStatus"_s, loopStatus()}});
    });
    connect(player, &Player::seeked, this, [this](qint64 position) {
        Q_EMIT Seeked(toMicroseconds(position));
    });
}

QString MediaPlayer2PlayerAdaptor::playbackStatus() const
{
    switch (m_player->playbackState()) {
    case Player::Playing:
        return u"Playing"_s;
    case Player::Paused:
        return u"Paused"_s;
    case Player::Stopped:
        break;
    }
    return u"Stopped"_s;
}

QString MediaPlayer2PlayerAdaptor::loopStatus() const
{
    switch (m_player->repeatMode()) {
    case Player::RepeatOne:
        return u"Track"_s;
    case Player::RepeatAll:
        return u"Playlist"_s;
    case Player::RepeatNone:
        break;
    }
    return u"None"_s;
}

void MediaPlayer2PlayerAdaptor::setLoopStatus(const QString &status)
{
    if (status == u"Track") {
        m_player->setRepeatMode(Player::RepeatOne);
    } else if (status == u"Playlist") {
        m_player->setRepeatMode(Player::RepeatAll);
    } else {
        m_player->setRepeatMode(Player::RepeatNone);
    }
}

double MediaPlayer2PlayerAdaptor::rate() const
{
    return 1.0;
}

void MediaPlayer2PlayerAdaptor::setRate(double rate)
{
    // Only normal speed is supported, and the spec says a rate of 0 means pause
    if (rate == 0.0) {
        m_player->pause();
    }
}

bool MediaPlayer2PlayerAdaptor::shuffle() const
{
    return m_player->shuffle();
}

void MediaPlayer2PlayerAdaptor::setShuffle(bool shuffle)
{
    m_player->setShuffle(shuffle);
}

QVariantMap MediaPlayer2PlayerAdaptor::metadata() const
{
    if (!m_player->hasTrack()) {
        return {{u"mpris:trackid"_s, QVariant::fromValue(currentTrackId())}};
    }

    const Track track = m_player->currentTrack();
    QVariantMap metadata{
        {u"mpris:trackid"_s, QVariant::fromValue(currentTrackId())},
        {u"mpris:length"_s, toMicroseconds(m_player->duration())},
        {u"xesam:url"_s, track.url.toString()},
        {u"xesam:title"_s, track.title},
    };
    if (!track.artist.isEmpty()) {
        metadata.insert(u"xesam:artist"_s, QStringList{track.artist});
    }
    if (!track.album.isEmpty()) {
        metadata.insert(u"xesam:album"_s, track.album);
    }
    if (!track.albumArtist.isEmpty()) {
        metadata.insert(u"xesam:albumArtist"_s, QStringList{track.albumArtist});
    }
    if (!track.genre.isEmpty()) {
        metadata.insert(u"xesam:genre"_s, QStringList{track.genre});
    }
    if (track.trackNumber > 0) {
        metadata.insert(u"xesam:trackNumber"_s, track.trackNumber);
    }
    if (track.discNumber > 0) {
        metadata.insert(u"xesam:discNumber"_s, track.discNumber);
    }
    if (!track.artworkUrl.isEmpty()) {
        metadata.insert(u"mpris:artUrl"_s, track.artworkUrl.toString());
    }
    return metadata;
}

double MediaPlayer2PlayerAdaptor::volume() const
{
    return m_player->volume();
}

void MediaPlayer2PlayerAdaptor::setVolume(double volume)
{
    m_player->setVolume(volume);
}

qlonglong MediaPlayer2PlayerAdaptor::position() const
{
    return toMicroseconds(m_player->position());
}

bool MediaPlayer2PlayerAdaptor::canGoNext() const
{
    return m_player->canGoNext();
}

bool MediaPlayer2PlayerAdaptor::canGoPrevious() const
{
    return m_player->canGoPrevious();
}

bool MediaPlayer2PlayerAdaptor::canPlay() const
{
    return m_player->hasTrack();
}

bool MediaPlayer2PlayerAdaptor::canPause() const
{
    return m_player->hasTrack();
}

bool MediaPlayer2PlayerAdaptor::canSeek() const
{
    return m_player->hasTrack();
}

bool MediaPlayer2PlayerAdaptor::canControl() const
{
    return true;
}

void MediaPlayer2PlayerAdaptor::Next()
{
    m_player->next();
}

void MediaPlayer2PlayerAdaptor::Previous()
{
    m_player->previous();
}

void MediaPlayer2PlayerAdaptor::Pause()
{
    m_player->pause();
}

void MediaPlayer2PlayerAdaptor::PlayPause()
{
    m_player->togglePlayPause();
}

void MediaPlayer2PlayerAdaptor::Stop()
{
    m_player->stop();
}

void MediaPlayer2PlayerAdaptor::Play()
{
    m_player->play();
}

void MediaPlayer2PlayerAdaptor::Seek(qlonglong offset)
{
    const qint64 position = m_player->position() + offset / 1000;
    // Seeking past the end acts like Next
    if (position > m_player->duration()) {
        m_player->next();
    } else {
        m_player->seek(position);
    }
}

void MediaPlayer2PlayerAdaptor::SetPosition(const QDBusObjectPath &trackId, qlonglong position)
{
    // Ignore requests meant for a track that is no longer current
    if (trackId != currentTrackId() || position < 0 || position > toMicroseconds(m_player->duration())) {
        return;
    }
    m_player->seek(position / 1000);
}

void MediaPlayer2PlayerAdaptor::OpenUri(const QString &uri)
{
    m_player->openUrls({QUrl::fromUserInput(uri)});
}

QDBusObjectPath MediaPlayer2PlayerAdaptor::currentTrackId() const
{
    if (!m_player->hasTrack()) {
        return QDBusObjectPath(u"/org/mpris/MediaPlayer2/TrackList/NoTrack"_s);
    }
    return QDBusObjectPath(u"/org/kde/encoremusic/track/%1"_s.arg(m_player->queue()->currentId()));
}

void MediaPlayer2PlayerAdaptor::notifyChanged(const QVariantMap &properties)
{
    QDBusMessage signal = QDBusMessage::createSignal(ObjectPath, u"org.freedesktop.DBus.Properties"_s, u"PropertiesChanged"_s);
    signal << PlayerInterface << properties << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
