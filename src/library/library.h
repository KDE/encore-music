// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "librarydata.h"

#include <QFutureWatcher>
#include <QObject>
#include <QtQmlIntegration>

class QQmlEngine;
class QJSEngine;

// The music in the user's music folders
class Library : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList folders READ folders NOTIFY foldersChanged)
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(int scannedFiles READ scannedFiles NOTIFY scannedFilesChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY changed)
    Q_PROPERTY(int albumCount READ albumCount NOTIFY changed)
    Q_PROPERTY(int artistCount READ artistCount NOTIFY changed)

public:
    static Library *instance();
    static Library *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    const LibraryData &data() const;

    QStringList folders() const;

    bool isScanning() const;
    int scannedFiles() const;

    int trackCount() const;
    int albumCount() const;
    int artistCount() const;

    Q_INVOKABLE Album album(const QString &key) const;
    Q_INVOKABLE Artist artist(const QString &key) const;

public Q_SLOTS:
    void rescan();
    void addFolder(const QUrl &url);
    void removeFolder(const QString &path);
    void cancel(); // cancel scan

Q_SIGNALS:
    void foldersChanged();
    void scanningChanged();
    void scannedFilesChanged();
    void changed(); // tracks, albums or artists changed

private:
    explicit Library(QObject *parent = nullptr);

    void setFolders(const QStringList &folders);
    void setData(const LibraryData &data);
    void setScannedFiles(int count);

    LibraryData m_data;
    QStringList m_folders;
    QFutureWatcher<LibraryData> m_scan;

    int m_scannedFiles = 0;
    bool m_rescanPending = false;
};
