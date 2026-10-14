// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "playqueue.h"
#include "track.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QObject>
#include <QtQmlIntegration>

class QQmlEngine;
class QJSEngine;

class Player : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(PlayQueue *queue READ queue CONSTANT)
    Q_PROPERTY(Track currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(bool hasTrack READ hasTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(RepeatMode repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY navigationChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    enum PlaybackState {
        Stopped,
        Playing,
        Paused,
    };
    Q_ENUM(PlaybackState)

    enum RepeatMode {
        RepeatNone,
        RepeatAll,
        RepeatOne,
    };
    Q_ENUM(RepeatMode)

    static Player *instance();
    static Player *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    PlayQueue *queue() const;
    Track currentTrack() const;
    bool hasTrack() const;

    PlaybackState playbackState() const;
    bool isPlaying() const;

    qint64 position() const; // ms
    qint64 duration() const;

    qreal volume() const;
    void setVolume(qreal volume);

    bool shuffle() const;
    void setShuffle(bool shuffle);

    RepeatMode repeatMode() const;
    void setRepeatMode(RepeatMode mode);

    bool canGoNext() const;
    bool canGoPrevious() const;

    // Whether files are being read to be queued
    bool isLoading() const;

    // Formats a time in milliseconds (ex. 1:02:45)
    Q_INVOKABLE static QString formatTime(qint64 milliseconds);

public Q_SLOTS:
    void play();
    void pause();
    void togglePlayPause();
    void stop();
    void next();
    void previous();
    void seek(qint64 position);
    void playIndex(int index);

    void openUrls(const QList<QUrl> &urls);
    void enqueue(const QList<QUrl> &urls);
    void playNext(const QList<QUrl> &urls);
    void playTracks(const QList<Track> &tracks, int startIndex, bool shuffle);

Q_SIGNALS:
    void currentTrackChanged();
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void shuffleChanged();
    void repeatModeChanged();
    void navigationChanged();
    void loadingChanged();
    void seeked(qint64 position);
    void errorOccurred(const QString &message);

private:
    enum class LoadAction {
        Replace,
        Append,
        InsertNext,
    };

    explicit Player(QObject *parent = nullptr);

    void load(const QList<QUrl> &urls, LoadAction action);
    void loadCurrentTrack();
    void handleMediaStatus(QMediaPlayer::MediaStatus status);
    void handleError(QMediaPlayer::Error error, const QString &message);

    bool advance();

    QMediaPlayer m_mediaPlayer;
    QAudioOutput m_audioOutput;
    PlayQueue *m_queue;
    RepeatMode m_repeatMode = RepeatNone;

    // Whether to start playing once the current track has loaded
    bool m_playWhenLoaded = false;

    int m_consecutiveErrors = 0;
    int m_pendingLoads = 0;
};
