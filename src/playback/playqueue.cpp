// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "playqueue.h"

#include <QRandomGenerator>

#include <algorithm>

PlayQueue::PlayQueue(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PlayQueue::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant PlayQueue::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const Track &track = m_entries.at(index.row()).track;
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return track.title;
    case TrackRole:
        return QVariant::fromValue(track);
    case ArtistRole:
        return track.artist;
    case AlbumRole:
        return track.album;
    case ArtworkUrlRole:
        return track.artworkUrl;
    case DurationRole:
        return track.duration;
    case IsCurrentRole:
        return index.row() == m_currentIndex;
    }
    return {};
}

QHash<int, QByteArray> PlayQueue::roleNames() const
{
    return {
        {TrackRole, "track"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {ArtworkUrlRole, "artworkUrl"},
        {DurationRole, "duration"},
        {IsCurrentRole, "isCurrent"},
    };
}

int PlayQueue::count() const
{
    return m_entries.size();
}

bool PlayQueue::isEmpty() const
{
    return m_entries.isEmpty();
}

int PlayQueue::currentIndex() const
{
    return m_currentIndex;
}

void PlayQueue::setCurrentIndex(int index)
{
    setCurrent(index, false);
}

Track PlayQueue::currentTrack() const
{
    return m_currentIndex >= 0 ? m_entries.at(m_currentIndex).track : Track{};
}

quint64 PlayQueue::currentId() const
{
    return m_currentIndex >= 0 ? m_entries.at(m_currentIndex).id : 0;
}

void PlayQueue::setTracks(const QList<Track> &tracks, int startIndex)
{
    beginResetModel();
    m_entries = makeEntries(tracks);
    m_currentIndex = m_entries.isEmpty() ? -1 : std::clamp(startIndex, 0, int(m_entries.size()) - 1);
    if (m_shuffled) {
        m_unshuffledEntries = m_entries;
        shuffleEntries();
    }
    endResetModel();

    Q_EMIT countChanged();
    Q_EMIT currentIndexChanged();
    Q_EMIT currentTrackChanged();
}

void PlayQueue::append(const QList<Track> &tracks)
{
    if (tracks.isEmpty()) {
        return;
    }

    const QList<Entry> entries = makeEntries(tracks);
    beginInsertRows({}, m_entries.size(), m_entries.size() + entries.size() - 1);
    m_entries.append(entries);
    if (m_shuffled) {
        m_unshuffledEntries.append(entries);
    }
    endInsertRows();
    Q_EMIT countChanged();
}

void PlayQueue::insertNext(const QList<Track> &tracks)
{
    if (tracks.isEmpty()) {
        return;
    }

    const QList<Entry> entries = makeEntries(tracks);
    const int position = m_currentIndex + 1;

    if (m_shuffled) {
        // Keep the new tracks after the current one in the original order too
        const quint64 currentId = this->currentId();
        const auto current = std::find_if(m_unshuffledEntries.begin(), m_unshuffledEntries.end(), [currentId](const Entry &entry) {
            return entry.id == currentId;
        });
        const qsizetype unshuffledPosition = current == m_unshuffledEntries.end() ? 0 : std::distance(m_unshuffledEntries.begin(), current) + 1;
        for (qsizetype i = 0; i < entries.size(); ++i) {
            m_unshuffledEntries.insert(unshuffledPosition + i, entries.at(i));
        }
    }

    beginInsertRows({}, position, position + entries.size() - 1);
    for (qsizetype i = 0; i < entries.size(); ++i) {
        m_entries.insert(position + i, entries.at(i));
    }
    endInsertRows();
    Q_EMIT countChanged();
}

void PlayQueue::remove(int index)
{
    if (index < 0 || index >= m_entries.size()) {
        return;
    }

    const quint64 id = m_entries.at(index).id;
    m_unshuffledEntries.removeIf([id](const Entry &entry) {
        return entry.id == id;
    });

    const bool removingCurrent = index == m_currentIndex;
    beginRemoveRows({}, index, index);
    m_entries.removeAt(index);
    endRemoveRows();
    Q_EMIT countChanged();

    if (index < m_currentIndex) {
        --m_currentIndex;
        Q_EMIT currentIndexChanged();
    } else if (removingCurrent) {
        // The next track takes its place
        m_currentIndex = -1;
        setCurrent(std::min(index, int(m_entries.size()) - 1), true);
    }
}

void PlayQueue::clear()
{
    setTracks({}, -1);
    m_unshuffledEntries.clear();
}

bool PlayQueue::isShuffled() const
{
    return m_shuffled;
}

void PlayQueue::setShuffled(bool shuffled)
{
    if (m_shuffled == shuffled) {
        return;
    }
    m_shuffled = shuffled;

    beginResetModel();
    if (shuffled) {
        m_unshuffledEntries = m_entries;
        shuffleEntries();
    } else {
        const quint64 currentId = this->currentId();
        m_entries = m_unshuffledEntries;
        m_unshuffledEntries.clear();
        const auto current = std::find_if(m_entries.cbegin(), m_entries.cend(), [currentId](const Entry &entry) {
            return entry.id == currentId;
        });
        m_currentIndex = current == m_entries.cend() ? (m_entries.isEmpty() ? -1 : 0) : std::distance(m_entries.cbegin(), current);
    }
    endResetModel();

    // The current track is the same, only its position changed
    Q_EMIT currentIndexChanged();
}

QList<PlayQueue::Entry> PlayQueue::makeEntries(const QList<Track> &tracks)
{
    QList<Entry> entries;
    entries.reserve(tracks.size());
    for (const Track &track : tracks) {
        entries.append({m_nextId++, track});
    }
    return entries;
}

void PlayQueue::shuffleEntries()
{
    if (m_currentIndex > 0) {
        m_entries.move(m_currentIndex, 0);
    }
    const auto first = m_currentIndex >= 0 ? m_entries.begin() + 1 : m_entries.begin();
    std::shuffle(first, m_entries.end(), *QRandomGenerator::global());
    m_currentIndex = m_entries.isEmpty() ? -1 : 0;
}

void PlayQueue::setCurrent(int index, bool forceTrackChanged)
{
    if (index < -1 || index >= m_entries.size()) {
        return;
    }
    if (index == m_currentIndex && !forceTrackChanged) {
        return;
    }

    const int previous = m_currentIndex;
    m_currentIndex = index;

    if (previous >= 0 && previous < m_entries.size()) {
        Q_EMIT dataChanged(this->index(previous), this->index(previous), {IsCurrentRole});
    }
    if (index >= 0) {
        Q_EMIT dataChanged(this->index(index), this->index(index), {IsCurrentRole});
    }
    Q_EMIT currentIndexChanged();
    Q_EMIT currentTrackChanged();
}
