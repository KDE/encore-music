// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "library.h"
#include "scanner.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QJSEngine>
#include <QPointer>
#include <QStandardPaths>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

static KConfigGroup libraryConfig()
{
    return KSharedConfig::openConfig()->group(u"Library"_s);
}

Library *Library::instance()
{
    static Library *library = new Library;
    return library;
}

Library *Library::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    Library *library = instance();
    QJSEngine::setObjectOwnership(library, QJSEngine::CppOwnership);
    return library;
}

Library::Library(QObject *parent)
    : QObject(parent)
{
    const QStringList defaultFolders = {QStandardPaths::writableLocation(QStandardPaths::MusicLocation)};
    m_folders = libraryConfig().readPathEntry(u"Folders"_s, defaultFolders);

    connect(&m_scan, &QFutureWatcher<LibraryData>::resultReadyAt, this, [this](int index) {
        setData(m_scan.resultAt(index));
    });
    connect(&m_scan, &QFutureWatcher<LibraryData>::finished, this, [this] {
        if (m_rescanPending) {
            m_rescanPending = false;
            rescan();
            return;
        }
        Q_EMIT scanningChanged();
    });
}

const LibraryData &Library::data() const
{
    return m_data;
}

QStringList Library::folders() const
{
    return m_folders;
}

bool Library::isScanning() const
{
    return m_scan.isRunning();
}

int Library::scannedFiles() const
{
    return m_scannedFiles;
}

int Library::trackCount() const
{
    return m_data.tracks.size();
}

int Library::albumCount() const
{
    return m_data.albums.size();
}

int Library::artistCount() const
{
    return m_data.artists.size();
}

Album Library::album(const QString &key) const
{
    const int index = m_data.albumIndexes.value(key, -1);
    return index >= 0 ? m_data.albums.at(index) : Album();
}

Artist Library::artist(const QString &key) const
{
    const int index = m_data.artistIndexes.value(key, -1);
    return index >= 0 ? m_data.artists.at(index) : Artist();
}

void Library::rescan()
{
    if (m_scan.isRunning()) {
        m_rescanPending = true;
        return;
    }

    const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDirectory);
    const QString databasePath = dataDirectory + u"/library.sqlite"_s;

    setScannedFiles(0);
    QPointer<Library> self(this);
    const auto progress = [self](int count) {
        QMetaObject::invokeMethod(
            self,
            [self, count] {
                if (self) {
                    self->setScannedFiles(count);
                }
            },
            Qt::QueuedConnection);
    };

    m_scan.setFuture(QtConcurrent::run(&Scanner::scan, databasePath, m_folders, progress));
    Q_EMIT scanningChanged();
}

void Library::addFolder(const QUrl &url)
{
    const QString path = QDir::cleanPath(url.toLocalFile());
    if (path.isEmpty() || m_folders.contains(path)) {
        return;
    }
    setFolders(m_folders + QStringList{path});
}

void Library::removeFolder(const QString &path)
{
    QStringList folders = m_folders;
    if (folders.removeAll(path) > 0) {
        setFolders(folders);
    }
}

void Library::cancel()
{
    m_rescanPending = false;
    m_scan.cancel();
}

void Library::setFolders(const QStringList &folders)
{
    m_folders = folders;
    KConfigGroup config = libraryConfig();
    config.writePathEntry(u"Folders"_s, m_folders);
    config.sync();
    Q_EMIT foldersChanged();
    rescan();
}

void Library::setData(const LibraryData &data)
{
    m_data = data;
    Q_EMIT changed();
}

void Library::setScannedFiles(int count)
{
    if (m_scannedFiles != count) {
        m_scannedFiles = count;
        Q_EMIT scannedFilesChanged();
    }
}
