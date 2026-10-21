// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "librarydata.h"

#include <QAbstractListModel>
#include <QtQmlIntegration>

// Songs in the library, can be filtered by album, artist or search text
class TrackModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString albumKey READ albumKey WRITE setAlbumKey NOTIFY albumKeyChanged)
    Q_PROPERTY(QString artistKey READ artistKey WRITE setArtistKey NOTIFY artistKeyChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(Album album READ album NOTIFY countChanged)
    Q_PROPERTY(Artist artist READ artist NOTIFY countChanged)

public:
    enum Roles {
        TrackRole = Qt::UserRole + 1,
        UrlRole,
        TitleRole,
        ArtistRole,
        AlbumRole,
        ArtworkUrlRole,
        DurationRole,
        TrackNumberRole,
        DiscNumberRole,
    };
    Q_ENUM(Roles)

    explicit TrackModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString albumKey() const;
    void setAlbumKey(const QString &key);

    QString artistKey() const;
    void setArtistKey(const QString &key);

    QString searchText() const;
    void setSearchText(const QString &text);

    Album album() const;
    Artist artist() const;

    Q_INVOKABLE void play(int index = 0);
    Q_INVOKABLE void shuffle();

Q_SIGNALS:
    void albumKeyChanged();
    void artistKeyChanged();
    void searchTextChanged();
    void countChanged();

private:
    void refresh();
    void setRows(const QList<int> &rows);
    QList<Track> tracks() const;

    QString m_albumKey;
    QString m_artistKey;
    QString m_searchText;

    QList<int> m_rows;
};
