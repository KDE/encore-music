// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "artworkcache.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QSaveFile>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

// Cap artwork at 1080p size
constexpr int MaximumSize = 1024;

static QString cacheDirectory()
{
    static const QString directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/covers"_s;
    return directory;
}

QUrl ArtworkCache::storeEmbedded(const QByteArray &imageData)
{
    if (imageData.isEmpty()) {
        return {};
    }

    // Identify image by hash so we coalesce the same image
    const QString key = QString::fromLatin1(QCryptographicHash::hash(imageData, QCryptographicHash::Sha1).toHex());
    const QString path = cacheDirectory() + u'/' + key + u".jpg"_s;
    if (QFileInfo::exists(path)) {
        return QUrl::fromLocalFile(path);
    }

    QImage image;
    if (!image.loadFromData(imageData)) {
        return {};
    }
    if (image.width() > MaximumSize || image.height() > MaximumSize) {
        image = image.scaled(MaximumSize, MaximumSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QDir().mkpath(cacheDirectory());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || !image.save(&file, "JPEG", 90) || !file.commit()) {
        return {};
    }
    return QUrl::fromLocalFile(path);
}

QUrl ArtworkCache::findFolderImage(const QString &directory)
{
    static const QStringList names = {u"cover"_s, u"folder"_s, u"front"_s, u"album"_s};
    static const QStringList extensions = {u"jpg"_s, u"jpeg"_s, u"png"_s, u"webp"_s};

    QStringList filters;
    for (const QString &extension : extensions) {
        filters.append(u"*."_s + extension);
    }

    const QFileInfoList images = QDir(directory).entryInfoList(filters, QDir::Files | QDir::Readable);
    for (const QString &name : names) {
        for (const QFileInfo &image : images) {
            if (image.completeBaseName().compare(name, Qt::CaseInsensitive) == 0) {
                return QUrl::fromLocalFile(image.absoluteFilePath());
            }
        }
    }
    return {};
}
