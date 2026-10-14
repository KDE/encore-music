// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QString>
#include <QUrl>
#include <QtQmlIntegration>

struct Track {
    Q_GADGET
    QML_VALUE_TYPE(track)

    Q_PROPERTY(QUrl url MEMBER url)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString artist MEMBER artist)
    Q_PROPERTY(QString album MEMBER album)
    Q_PROPERTY(QString albumArtist MEMBER albumArtist)
    Q_PROPERTY(QString genre MEMBER genre)
    Q_PROPERTY(int trackNumber MEMBER trackNumber)
    Q_PROPERTY(int discNumber MEMBER discNumber)
    Q_PROPERTY(int year MEMBER year)
    Q_PROPERTY(qint64 duration MEMBER duration)
    Q_PROPERTY(QUrl artworkUrl MEMBER artworkUrl)
    Q_PROPERTY(bool valid READ isValid)

public:
    QUrl url;
    QString title;
    QString artist;
    QString album;
    QString albumArtist;
    QString genre;
    int trackNumber = 0;
    int discNumber = 0;
    int year = 0;
    qint64 duration = 0; // ms
    QUrl artworkUrl;
    qint64 modified = 0;

    bool isValid() const
    {
        return url.isValid();
    }

    bool operator==(const Track &other) const = default;
};
