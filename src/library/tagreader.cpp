// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "tagreader.h"
#include "artworkcache.h"

#include <KFileMetaData/EmbeddedImageData>
#include <KFileMetaData/Extractor>
#include <KFileMetaData/Properties>
#include <KFileMetaData/SimpleExtractionResult>

#include <QCollator>
#include <QDirIterator>
#include <QFileInfo>

#include <algorithm>

using namespace Qt::StringLiterals;
using namespace KFileMetaData;

static QString joinedValues(const PropertyMultiMap &properties, Property::Property property)
{
    QStringList values;
    const QList<QVariant> variants = properties.values(property);
    for (auto it = variants.crbegin(); it != variants.crend(); ++it) {
        const QString value = it->toString().trimmed();
        if (!value.isEmpty() && !values.contains(value)) {
            values.append(value);
        }
    }
    return values.join(u", "_s);
}

Track TagReader::read(const QString &path)
{
    Track track;
    track.url = QUrl::fromLocalFile(path);

    const QString mimeType = m_mimeDatabase.mimeTypeForFile(path).name();
    SimpleExtractionResult result(path, mimeType, ExtractionResult::ExtractMetaData | ExtractionResult::ExtractImageData);

    const QList<Extractor *> extractors = m_extractors.fetchExtractors(mimeType);
    for (Extractor *extractor : extractors) {
        extractor->extract(&result);
        if (!result.properties().isEmpty()) {
            break;
        }
    }

    const PropertyMultiMap properties = result.properties();
    track.title = properties.value(Property::Title).toString().trimmed();
    track.artist = joinedValues(properties, Property::Artist);
    track.album = properties.value(Property::Album).toString().trimmed();
    track.albumArtist = joinedValues(properties, Property::AlbumArtist);
    track.genre = joinedValues(properties, Property::Genre);
    track.trackNumber = properties.value(Property::TrackNumber).toInt();
    track.discNumber = properties.value(Property::DiscNumber).toInt();
    track.year = properties.value(Property::ReleaseYear).toInt();
    track.duration = qRound64(properties.value(Property::Duration).toDouble() * 1000);

    const QFileInfo fileInfo(path);
    track.modified = fileInfo.lastModified().toSecsSinceEpoch();
    if (track.title.isEmpty()) {
        track.title = fileInfo.completeBaseName();
    }

    const auto images = result.imageData();
    QByteArray image = images.value(EmbeddedImageData::FrontCover);
    if (image.isEmpty() && !images.isEmpty()) {
        image = images.first();
    }
    track.artworkUrl = ArtworkCache::storeEmbedded(image);

    if (track.artworkUrl.isEmpty()) {
        const QString directory = fileInfo.absolutePath();
        auto it = m_folderImages.constFind(directory);
        if (it == m_folderImages.constEnd()) {
            it = m_folderImages.insert(directory, ArtworkCache::findFolderImage(directory));
        }
        track.artworkUrl = *it;
    }

    return track;
}

QStringList TagReader::findAudioFiles(const QList<QUrl> &urls)
{
    QCollator collator;
    collator.setNumericMode(true);

    QStringList files;
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        const QFileInfo info(path);

        if (info.isDir()) {
            QStringList found;
            QDirIterator it(path, QDir::Files | QDir::Readable, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            while (it.hasNext()) {
                const QString file = it.next();
                if (isAudioFile(file)) {
                    found.append(file);
                }
            }
            // Numeric so that "2 Song" comes before "10 Song"
            std::sort(found.begin(), found.end(), [&collator](const QString &a, const QString &b) {
                return collator.compare(a, b) < 0;
            });
            files.append(found);
        } else if (info.isFile() && isAudioFile(path)) {
            files.append(path);
        }
    }
    return files;
}

bool TagReader::isAudioFile(const QString &path)
{
    return m_mimeDatabase.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name().startsWith(u"audio/"_s);
}
