// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "artistmodel.h"
#include "library.h"

ArtistModel::ArtistModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(Library::instance(), &Library::changed, this, &ArtistModel::refresh);
    refresh();
}

int ArtistModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_count;
}

QVariant ArtistModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const QList<Artist> &artists = Library::instance()->data().artists;
    if (index.row() >= artists.size()) {
        return {};
    }
    const Artist &artist = artists.at(index.row());
    switch (role) {
    case KeyRole:
        return artist.key;
    case Qt::DisplayRole:
    case NameRole:
        return artist.name;
    case ArtworkUrlRole:
        return artist.artworkUrl;
    case AlbumCountRole:
        return artist.albumCount();
    case TrackCountRole:
        return artist.trackCount;
    }
    return {};
}

QHash<int, QByteArray> ArtistModel::roleNames() const
{
    return {
        {KeyRole, "key"},
        {NameRole, "name"},
        {ArtworkUrlRole, "artworkUrl"},
        {AlbumCountRole, "albumCount"},
        {TrackCountRole, "trackCount"},
    };
}

void ArtistModel::refresh()
{
    const int count = Library::instance()->data().artists.size();
    if (m_count == 0 && count > 0) {
        beginInsertRows(QModelIndex(), 0, count - 1);
        m_count = count;
        endInsertRows();
    } else if (m_count > 0 && count == 0) {
        beginRemoveRows(QModelIndex(), 0, m_count - 1);
        m_count = 0;
        endRemoveRows();
    } else {
        beginResetModel();
        m_count = count;
        endResetModel();
    }
    Q_EMIT countChanged();
}
