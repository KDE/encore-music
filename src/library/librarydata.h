// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "track.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QUrl>
#include <QtQmlIntegration>

struct Album {
    Q_GADGET
    QML_VALUE_TYPE(album)

    Q_PROPERTY(QString key MEMBER key)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString artist MEMBER artist)
    Q_PROPERTY(QString artistKey MEMBER artistKey)
    Q_PROPERTY(QString genre MEMBER genre)
    Q_PROPERTY(int year MEMBER year)
    Q_PROPERTY(QUrl artworkUrl MEMBER artworkUrl)
    Q_PROPERTY(qint64 duration MEMBER duration)
    Q_PROPERTY(int trackCount READ trackCount)
    Q_PROPERTY(int discCount MEMBER discCount)
    Q_PROPERTY(bool valid READ isValid)

public:
    QString key;
    QString title;
    QString artist;
    QString artistKey;
    QString genre;

    int year = 0;
    QUrl artworkUrl;
    qint64 duration = 0; // ms
    qint64 modified = 0; // last modified
    int discCount = 0;

    // Indexes into LibraryData::tracks, in playing order
    QList<int> tracks;

    int trackCount() const
    {
        return tracks.size();
    }

    bool isValid() const
    {
        return !key.isEmpty();
    }
};

// Album artist
struct Artist {
    Q_GADGET
    QML_VALUE_TYPE(artist)

    Q_PROPERTY(QString key MEMBER key)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QUrl artworkUrl MEMBER artworkUrl)
    Q_PROPERTY(int albumCount READ albumCount)
    Q_PROPERTY(int trackCount MEMBER trackCount)
    Q_PROPERTY(bool valid READ isValid)

public:
    QString key;
    QString name;
    QUrl artworkUrl;
    int trackCount = 0;

    // Indexes into LibraryData::albums, oldest first
    QList<int> albums;

    int albumCount() const
    {
        return albums.size();
    }

    bool isValid() const
    {
        return !key.isEmpty();
    }
};

// Indexed music library
struct LibraryData {
    QList<Track> tracks;
    QList<Album> albums;
    QList<Artist> artists;

    QList<int> tracksByTitle; // Indexes into tracks, by title
    QList<int> albumsByModified; // Indexes into albums, newest first

    QHash<QString, int> albumIndexes;
    QHash<QString, int> artistIndexes;

    static LibraryData build(QList<Track> tracks);
};
