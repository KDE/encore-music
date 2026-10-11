// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace ArtworkCache
{
// Saves embedded artwork to the cache and returns its file URL
QUrl storeEmbedded(const QByteArray &imageData);

// Returns the cover image in a folder, or an empty URL if there is none
QUrl findFolderImage(const QString &directory);
}
