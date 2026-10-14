// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "librarydata.h"

#include <KLocalizedString>

#include <QCollator>
#include <QFileInfo>
#include <QSet>

#include <algorithm>
#include <numeric>

using namespace Qt::StringLiterals;

// A track and the album it is grouped into
struct Entry {
    Track track;
    QString artistName;
    QString artistKey;
    QString albumKey;
};

static QString foldedKey(const QString &text)
{
    return text.trimmed().toCaseFolded();
}

// Compilation "album" - Tracks without an album artist tag, from one album in one folder, by different artists
static QSet<QString> findCompilations(const QList<Track> &tracks)
{
    QHash<QString, QSet<QString>> artists;
    for (const Track &track : tracks) {
        if (track.albumArtist.isEmpty() && !track.album.isEmpty()) {
            const QString key = foldedKey(track.album) + u'\x1f' + QFileInfo(track.url.toLocalFile()).path();
            artists[key].insert(foldedKey(track.artist));
        }
    }

    QSet<QString> compilations;
    for (auto it = artists.cbegin(); it != artists.cend(); ++it) {
        if (it->size() > 1) {
            compilations.insert(it.key());
        }
    }
    return compilations;
}

LibraryData LibraryData::build(QList<Track> tracks)
{
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    const auto lessThan = [&collator](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    };

    const QSet<QString> compilations = findCompilations(tracks);
    const QString variousArtists = i18nc("@info the artist of a compilation album", "Various Artists");
    const QString unknownArtist = i18n("Unknown artist");
    const QString unknownAlbum = i18n("Unknown album");

    // Choose each track's album artist, which groups it into an album
    QList<Entry> entries;
    entries.reserve(tracks.size());
    for (Track &track : tracks) {
        Entry entry;
        if (!track.albumArtist.isEmpty()) {
            entry.artistName = track.albumArtist;
        } else if (!track.album.isEmpty() && compilations.contains(foldedKey(track.album) + u'\x1f' + QFileInfo(track.url.toLocalFile()).path())) {
            entry.artistName = variousArtists;
        } else if (!track.artist.isEmpty()) {
            entry.artistName = track.artist;
        } else {
            entry.artistName = unknownArtist;
        }
        entry.artistKey = foldedKey(entry.artistName);
        entry.albumKey = entry.artistKey + u'\x1f' + foldedKey(track.album);
        entry.track = std::move(track);
        entries.append(std::move(entry));
    }

    // Each album's tracks together, in playing order
    std::sort(entries.begin(), entries.end(), [&collator](const Entry &a, const Entry &b) {
        if (a.albumKey != b.albumKey) {
            return a.albumKey < b.albumKey;
        }
        if (a.track.discNumber != b.track.discNumber) {
            return a.track.discNumber < b.track.discNumber;
        }
        if (a.track.trackNumber != b.track.trackNumber) {
            return a.track.trackNumber < b.track.trackNumber;
        }
        return collator.compare(a.track.url.path(), b.track.url.path()) < 0;
    });

    LibraryData data;
    data.tracks.reserve(entries.size());

    // Populate tracks list and album metadata
    for (int i = 0; i < entries.size(); ++i) {
        const Entry &entry = entries.at(i);
        const Track &track = entry.track;

        if (data.albums.isEmpty() || data.albums.constLast().key != entry.albumKey) {
            Album album;
            album.key = entry.albumKey;
            album.title = track.album.isEmpty() ? unknownAlbum : track.album;
            album.artist = entry.artistName;
            album.artistKey = entry.artistKey;
            data.albums.append(album);
        }

        Album &album = data.albums.last();
        // The tracks are in disc order, so a new disc starts where the number changes
        if (album.tracks.isEmpty() || data.tracks.constLast().discNumber != track.discNumber) {
            ++album.discCount;
        }
        album.tracks.append(data.tracks.size());
        album.duration += track.duration;
        album.modified = std::max(album.modified, track.modified);
        if (album.year == 0) {
            album.year = track.year;
        }
        if (album.genre.isEmpty()) {
            album.genre = track.genre;
        }
        if (album.artworkUrl.isEmpty()) {
            album.artworkUrl = track.artworkUrl;
        }

        data.tracks.append(track);
    }

    // Sort albums by title
    std::sort(data.albums.begin(), data.albums.end(), [&collator](const Album &a, const Album &b) {
        const int order = collator.compare(a.title, b.title);
        return order != 0 ? order < 0 : collator.compare(a.artist, b.artist) < 0;
    });

    // Index the albums by key, and gather them under their artists
    for (int i = 0; i < data.albums.size(); ++i) {
        const Album &album = data.albums.at(i);
        data.albumIndexes.insert(album.key, i);

        auto artistIndex = data.artistIndexes.constFind(album.artistKey);
        if (artistIndex == data.artistIndexes.cend()) {
            Artist artist;
            artist.key = album.artistKey;
            artist.name = album.artist;
            data.artists.append(artist);
            artistIndex = data.artistIndexes.insert(album.artistKey, data.artists.size() - 1);
        }

        Artist &artist = data.artists[*artistIndex];
        artist.albums.append(i);
        artist.trackCount += album.trackCount();
    }

    // Sort each artist's albums, oldest first
    for (Artist &artist : data.artists) {
        std::sort(artist.albums.begin(), artist.albums.end(), [&data, &lessThan](int a, int b) {
            const Album &first = data.albums.at(a);
            const Album &second = data.albums.at(b);
            if (first.year != second.year) {
                return first.year < second.year;
            }
            return lessThan(first.title, second.title);
        });

        // The newest album's artwork stands for the artist
        for (auto it = artist.albums.crbegin(); it != artist.albums.crend(); ++it) {
            if (!data.albums.at(*it).artworkUrl.isEmpty()) {
                artist.artworkUrl = data.albums.at(*it).artworkUrl;
                break;
            }
        }
    }

    // Sort artists by name
    std::sort(data.artists.begin(), data.artists.end(), [&lessThan](const Artist &a, const Artist &b) {
        return lessThan(a.name, b.name);
    });

    // Re-index
    data.artistIndexes.clear();
    for (int i = 0; i < data.artists.size(); ++i) {
        data.artistIndexes.insert(data.artists.at(i).key, i);
    }

    data.tracksByTitle.resize(data.tracks.size());
    std::iota(data.tracksByTitle.begin(), data.tracksByTitle.end(), 0);
    std::sort(data.tracksByTitle.begin(), data.tracksByTitle.end(), [&data, &collator](int a, int b) {
        const Track &first = data.tracks.at(a);
        const Track &second = data.tracks.at(b);
        const int order = collator.compare(first.title, second.title);
        return order != 0 ? order < 0 : collator.compare(first.artist, second.artist) < 0;
    });

    // Sort albums newest first
    data.albumsByModified.resize(data.albums.size());
    std::iota(data.albumsByModified.begin(), data.albumsByModified.end(), 0);
    std::stable_sort(data.albumsByModified.begin(), data.albumsByModified.end(), [&data](int a, int b) {
        return data.albums.at(a).modified > data.albums.at(b).modified;
    });

    return data;
}
