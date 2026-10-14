// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "librarydata.h"

#include <QPromise>
#include <QStringList>

#include <functional>

namespace Scanner
{
// Find audio files and populate DB
void scan(QPromise<LibraryData> &promise, const QString &databasePath, const QStringList &folders, const std::function<void(int)> &progress);
}
