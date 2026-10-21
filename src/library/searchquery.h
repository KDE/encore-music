// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QStringList>

class SearchQuery
{
public:
    explicit SearchQuery(const QString &text);

    bool isEmpty() const;
    bool matches(const QStringList &fields) const;

private:
    QStringList m_words;
};
