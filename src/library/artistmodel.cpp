// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "artistmodel.h"
#include "library.h"
#include "searchquery.h"

#include <numeric>

ArtistModel::ArtistModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(Library::instance(), &Library::changed, this, &ArtistModel::refresh);
    refresh();
}

int ArtistModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant ArtistModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const Artist &artist = Library::instance()->data().artists.at(m_rows.at(index.row()));
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

QString ArtistModel::searchText() const
{
    return m_searchText;
}

void ArtistModel::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }
    m_searchText = text;
    Q_EMIT searchTextChanged();
    refresh();
}

void ArtistModel::refresh()
{
    const QList<Artist> &artists = Library::instance()->data().artists;

    QList<int> rows(artists.size());
    std::iota(rows.begin(), rows.end(), 0);

    const SearchQuery query(m_searchText);
    if (!query.isEmpty()) {
        rows.removeIf([&artists, &query](int row) {
            return !query.matches({artists.at(row).name});
        });
    }
    setRows(rows);
}

void ArtistModel::setRows(const QList<int> &rows)
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
