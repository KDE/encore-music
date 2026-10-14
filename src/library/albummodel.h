// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QtQmlIntegration>

class AlbumModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString artistKey READ artistKey WRITE setArtistKey NOTIFY artistKeyChanged)

    Q_PROPERTY(SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        KeyRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        ArtistKeyRole,
        YearRole,
        ArtworkUrlRole,
        TrackCountRole,
    };
    Q_ENUM(Roles)

    enum SortOrder {
        Title,
        RecentlyAdded,
    };
    Q_ENUM(SortOrder)

    explicit AlbumModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString artistKey() const;
    void setArtistKey(const QString &key);

    SortOrder sortOrder() const;
    void setSortOrder(SortOrder order);

    // -1 for infinite
    int limit() const;
    void setLimit(int limit);

Q_SIGNALS:
    void artistKeyChanged();
    void sortOrderChanged();
    void limitChanged();
    void countChanged();

private:
    void refresh();
    void setRows(const QList<int> &rows);

    QString m_artistKey;
    SortOrder m_sortOrder = Title;
    int m_limit = -1;

    QList<int> m_rows;
};
