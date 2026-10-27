// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "scanner.h"
#include "tagreader.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(SCANNER, "org.kde.encoremusic.scanner", QtWarningMsg)

// Bump when the table changes, to start again with an empty one
constexpr int SCHEMA_VERSION = 1;

constexpr int PROGRESS_REPORT_INTERVAL = 25;

struct CachedTrack {
    Track track;
    qint64 size = 0;
};

static bool prepareSchema(QSqlDatabase &database)
{
    QSqlQuery query(database);
    query.exec(u"PRAGMA user_version"_s);
    const int version = query.next() ? query.value(0).toInt() : 0;
    if (version == SCHEMA_VERSION) {
        return true;
    }

    const QStringList statements = {
        u"DROP TABLE IF EXISTS tracks"_s,
        u"CREATE TABLE tracks ("
        "path TEXT PRIMARY KEY, modified INTEGER, size INTEGER, title TEXT, artist TEXT, album TEXT, "
        "album_artist TEXT, genre TEXT, track_number INTEGER, disc_number INTEGER, year INTEGER, "
        "duration INTEGER, artwork TEXT)"_s,
        u"PRAGMA user_version = %1"_s.arg(SCHEMA_VERSION),
    };
    for (const QString &statement : statements) {
        if (!query.exec(statement)) {
            qCWarning(SCANNER) << "Could not create the library database:" << query.lastError().text();
            return false;
        }
    }
    return true;
}

static QHash<QString, CachedTrack> readTracks(QSqlDatabase &database)
{
    QHash<QString, CachedTrack> tracks;
    QSqlQuery query(database);
    query.setForwardOnly(true);
    if (!query.exec(u"SELECT path, modified, size, title, artist, album, album_artist, genre, track_number, disc_number, year, duration, artwork "
                    "FROM tracks"_s)) {
        qCWarning(SCANNER) << "Could not read the library database:" << query.lastError().text();
        return tracks;
    }

    while (query.next()) {
        CachedTrack cached;
        const QString path = query.value(0).toString();
        cached.track.url = QUrl::fromLocalFile(path);
        cached.track.modified = query.value(1).toLongLong();
        cached.size = query.value(2).toLongLong();
        cached.track.title = query.value(3).toString();
        cached.track.artist = query.value(4).toString();
        cached.track.album = query.value(5).toString();
        cached.track.albumArtist = query.value(6).toString();
        cached.track.genre = query.value(7).toString();
        cached.track.trackNumber = query.value(8).toInt();
        cached.track.discNumber = query.value(9).toInt();
        cached.track.year = query.value(10).toInt();
        cached.track.duration = query.value(11).toLongLong();
        cached.track.artworkUrl = QUrl(query.value(12).toString());
        tracks.insert(path, cached);
    }
    return tracks;
}

static bool isInFolders(const QString &path, const QStringList &folders)
{
    return std::any_of(folders.cbegin(), folders.cend(), [&path](const QString &folder) {
        return path.startsWith(folder) && path.size() > folder.size() && path.at(folder.size()) == u'/';
    });
}

// Cached artwork can be removed by clearing the cache, and folder images can be deleted
static bool artworkExists(const Track &track)
{
    return track.artworkUrl.isEmpty() || QFileInfo::exists(track.artworkUrl.toLocalFile());
}

void Scanner::scan(QPromise<LibraryData> &promise, const QString &databasePath, const QStringList &folders, const std::function<void(int)> &progress)
{
    QStringList cleanFolders;
    for (const QString &folder : folders) {
        cleanFolders.append(QDir::cleanPath(folder));
    }

    // A connection belongs to the thread that made it, so each scan makes its own
    const QString connectionName = u"scanner"_s;
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(u"QSQLITE"_s, connectionName);
        database.setDatabaseName(databasePath);
        const bool open = database.open() && prepareSchema(database);
        if (!open) {
            qCWarning(SCANNER) << "Could not open the library database" << databasePath << database.lastError().text();
        }

        const QHash<QString, CachedTrack> cached = open ? readTracks(database) : QHash<QString, CachedTrack>();

        // Show what was there last time while checking for changes
        if (!cached.isEmpty()) {
            QList<Track> tracks;
            tracks.reserve(cached.size());
            for (auto it = cached.cbegin(); it != cached.cend(); ++it) {
                if (isInFolders(it.key(), cleanFolders)) {
                    tracks.append(it->track);
                }
            }
            promise.addResult(LibraryData::build(tracks));
        }

        QList<QUrl> folderUrls;
        for (const QString &folder : std::as_const(cleanFolders)) {
            folderUrls.append(QUrl::fromLocalFile(folder));
        }

        TagReader reader;
        const QStringList files = reader.findAudioFiles(folderUrls);

        QSqlQuery insert(database);
        insert.prepare(
            u"INSERT OR REPLACE INTO tracks "
            "(path, modified, size, title, artist, album, album_artist, genre, track_number, disc_number, year, duration, artwork) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"_s);
        database.transaction();

        QList<Track> tracks;
        tracks.reserve(files.size());
        QSet<QString> found;
        found.reserve(files.size());
        bool changed = false;

        for (const QString &file : files) {
            if (promise.isCanceled()) {
                break;
            }

            // Folders can overlap, such as a folder and one inside it
            if (found.contains(file)) {
                continue;
            }
            found.insert(file);

            const QFileInfo info(file);
            const qint64 modified = info.lastModified().toSecsSinceEpoch();
            const auto cachedTrack = cached.constFind(file);
            if (cachedTrack != cached.cend() && cachedTrack->track.modified == modified && cachedTrack->size == info.size()
                && artworkExists(cachedTrack->track)) {
                tracks.append(cachedTrack->track);
            } else {
                const Track track = reader.read(file);
                tracks.append(track);
                changed = true;

                insert.addBindValue(file);
                insert.addBindValue(track.modified);
                insert.addBindValue(info.size());
                insert.addBindValue(track.title);
                insert.addBindValue(track.artist);
                insert.addBindValue(track.album);
                insert.addBindValue(track.albumArtist);
                insert.addBindValue(track.genre);
                insert.addBindValue(track.trackNumber);
                insert.addBindValue(track.discNumber);
                insert.addBindValue(track.year);
                insert.addBindValue(track.duration);
                insert.addBindValue(track.artworkUrl.toString());
                if (!insert.exec()) {
                    qCWarning(SCANNER) << "Could not store" << file << insert.lastError().text();
                }
            }

            if (found.size() % PROGRESS_REPORT_INTERVAL == 0) {
                progress(found.size());
            }
        }

        // Forget files that are gone, unless the scan stopped before reaching them
        if (!promise.isCanceled()) {
            QSqlQuery remove(database);
            remove.prepare(u"DELETE FROM tracks WHERE path = ?"_s);
            for (auto it = cached.cbegin(); it != cached.cend(); ++it) {
                if (!found.contains(it.key())) {
                    remove.addBindValue(it.key());
                    remove.exec();
                    changed = true;
                }
            }
        }

        database.commit();
        progress(found.size());

        // Only report the scan when it differs from the cache, which was reported already
        if (!promise.isCanceled() && (changed || cached.isEmpty())) {
            promise.addResult(LibraryData::build(tracks));
        }
    }
    QSqlDatabase::removeDatabase(connectionName);
}
