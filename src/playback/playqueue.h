// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "track.h"

#include <QAbstractListModel>
#include <QtQmlIntegration>

class PlayQueue : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use Player.queue")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

public:
    enum Roles {
        TrackRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        ArtworkUrlRole,
        DurationRole,
        IsCurrentRole,
    };
    Q_ENUM(Roles)

    explicit PlayQueue(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;
    bool isEmpty() const;

    // -1 when nothing is selected
    int currentIndex() const;
    void setCurrentIndex(int index);

    Track currentTrack() const;
    quint64 currentId() const;

    // Replaces the queue, starting at startIndex
    void setTracks(const QList<Track> &tracks, int startIndex);
    void append(const QList<Track> &tracks);
    void insertNext(const QList<Track> &tracks);
    void remove(int index);
    void clear();

    bool isShuffled() const;
    void setShuffled(bool shuffled);

Q_SIGNALS:
    void countChanged();
    void currentIndexChanged();
    void currentTrackChanged();

private:
    struct Entry {
        quint64 id;
        Track track;
    };

    QList<Entry> makeEntries(const QList<Track> &tracks);
    // Moves the current entry to the front and shuffles everything after it
    void shuffleEntries();
    void setCurrent(int index, bool forceTrackChanged);

    QList<Entry> m_entries; // Shuffled order
    QList<Entry> m_unshuffledEntries; // The order before shuffling, while shuffled

    int m_currentIndex = -1;
    bool m_shuffled = false;
    quint64 m_nextId = 1;
};
