// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "albummodel.h"
#include "library.h"

#include <numeric>

AlbumModel::AlbumModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(Library::instance(), &Library::changed, this, &AlbumModel::refresh);
    refresh();
}

int AlbumModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant AlbumModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const QList<Album> &albums = Library::instance()->data().albums;
    int row = m_rows.at(index.row());
    if (row >= albums.size()) {
        return {};
    }
    const Album &album = albums.at(row);
    switch (role) {
    case KeyRole:
        return album.key;
    case Qt::DisplayRole:
    case TitleRole:
        return album.title;
    case ArtistRole:
        return album.artist;
    case ArtistKeyRole:
        return album.artistKey;
    case YearRole:
        return album.year;
    case ArtworkUrlRole:
        return album.artworkUrl;
    case TrackCountRole:
        return album.trackCount();
    }
    return {};
}

QHash<int, QByteArray> AlbumModel::roleNames() const
{
    return {
        {KeyRole, "key"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {ArtistKeyRole, "artistKey"},
        {YearRole, "year"},
        {ArtworkUrlRole, "artworkUrl"},
        {TrackCountRole, "trackCount"},
    };
}

QString AlbumModel::artistKey() const
{
    return m_artistKey;
}

void AlbumModel::setArtistKey(const QString &key)
{
    if (m_artistKey == key) {
        return;
    }
    m_artistKey = key;
    Q_EMIT artistKeyChanged();
    refresh();
}

AlbumModel::SortOrder AlbumModel::sortOrder() const
{
    return m_sortOrder;
}

void AlbumModel::setSortOrder(SortOrder order)
{
    if (m_sortOrder == order) {
        return;
    }
    m_sortOrder = order;
    Q_EMIT sortOrderChanged();
    refresh();
}

int AlbumModel::limit() const
{
    return m_limit;
}

void AlbumModel::setLimit(int limit)
{
    if (m_limit == limit) {
        return;
    }
    m_limit = limit;
    Q_EMIT limitChanged();
    refresh();
}

void AlbumModel::refresh()
{
    const LibraryData &data = Library::instance()->data();

    QList<int> rows;
    if (!m_artistKey.isEmpty()) {
        rows = Library::instance()->artist(m_artistKey).albums;
        if (m_sortOrder == RecentlyAdded) {
            std::stable_sort(rows.begin(), rows.end(), [&data](int a, int b) {
                return data.albums.at(a).modified > data.albums.at(b).modified;
            });
        }
    } else if (m_sortOrder == RecentlyAdded) {
        rows = data.albumsByModified;
    } else {
        rows.resize(data.albums.size());
        std::iota(rows.begin(), rows.end(), 0);
    }
    if (m_limit >= 0 && rows.size() > m_limit) {
        rows.resize(m_limit);
    }
    setRows(rows);
}

void AlbumModel::setRows(const QList<int> &rows)
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
