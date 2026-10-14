// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "trackmodel.h"
#include "library.h"
#include "player.h"

TrackModel::TrackModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(Library::instance(), &Library::changed, this, &TrackModel::refresh);
    refresh();
}

int TrackModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant TrackModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const QList<Track> &tracks = Library::instance()->data().tracks;
    const int row = m_rows.at(index.row());
    if (row >= tracks.size()) {
        return {};
    }
    const Track &track = tracks.at(row);
    switch (role) {
    case TrackRole:
        return QVariant::fromValue(track);
    case UrlRole:
        return track.url;
    case Qt::DisplayRole:
    case TitleRole:
        return track.title;
    case ArtistRole:
        return track.artist;
    case AlbumRole:
        return track.album;
    case ArtworkUrlRole:
        return track.artworkUrl;
    case DurationRole:
        return track.duration;
    case TrackNumberRole:
        return track.trackNumber;
    case DiscNumberRole:
        return track.discNumber;
    }
    return {};
}

QHash<int, QByteArray> TrackModel::roleNames() const
{
    return {
        {TrackRole, "track"},
        {UrlRole, "url"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {ArtworkUrlRole, "artworkUrl"},
        {DurationRole, "duration"},
        {TrackNumberRole, "trackNumber"},
        {DiscNumberRole, "discNumber"},
    };
}

QString TrackModel::albumKey() const
{
    return m_albumKey;
}

void TrackModel::setAlbumKey(const QString &key)
{
    if (m_albumKey == key) {
        return;
    }
    m_albumKey = key;
    Q_EMIT albumKeyChanged();
    refresh();
}

QString TrackModel::artistKey() const
{
    return m_artistKey;
}

void TrackModel::setArtistKey(const QString &key)
{
    if (m_artistKey == key) {
        return;
    }
    m_artistKey = key;
    Q_EMIT artistKeyChanged();
    refresh();
}

Album TrackModel::album() const
{
    return Library::instance()->album(m_albumKey);
}

Artist TrackModel::artist() const
{
    return Library::instance()->artist(m_artistKey);
}

void TrackModel::play(int index)
{
    Player::instance()->playTracks(tracks(), index, false);
}

void TrackModel::shuffle()
{
    Player::instance()->playTracks(tracks(), 0, true);
}

void TrackModel::refresh()
{
    const LibraryData &data = Library::instance()->data();

    QList<int> rows;
    if (!m_albumKey.isEmpty()) {
        rows = album().tracks;
    } else if (!m_artistKey.isEmpty()) {
        for (int albumIndex : artist().albums) {
            rows.append(data.albums.at(albumIndex).tracks);
        }
    } else {
        rows = data.tracksByTitle;
    }
    setRows(rows);
}

QList<Track> TrackModel::tracks() const
{
    const LibraryData &data = Library::instance()->data();
    QList<Track> tracks;
    tracks.reserve(m_rows.size());
    for (int row : m_rows) {
        tracks.append(data.tracks.at(row));
    }
    return tracks;
}

void TrackModel::setRows(const QList<int> &rows)
{
    if (m_rows.isEmpty() && !rows.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, rows.size() - 1);
        m_rows = rows;
        endInsertRows();
    } else if (!m_rows.isEmpty() && rows.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_rows.size() - 1);
        m_rows.clear();
        endRemoveRows();
    } else {
        beginResetModel();
        m_rows = rows;
        endResetModel();
    }
    Q_EMIT countChanged();
}
