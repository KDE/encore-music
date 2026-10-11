// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "track.h"

#include <KFileMetaData/ExtractorCollection>

#include <QHash>
#include <QList>
#include <QMimeDatabase>

// Reads tags and artwork from audio files
class TagReader
{
public:
    Track read(const QString &path);

    QStringList findAudioFiles(const QList<QUrl> &urls);

private:
    bool isAudioFile(const QString &path);

    KFileMetaData::ExtractorCollection m_extractors;
    QMimeDatabase m_mimeDatabase;
    QHash<QString, QUrl> m_folderImages;
};
