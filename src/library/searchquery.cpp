// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "searchquery.h"

#include <algorithm>

static QString fold(const QString &text)
{
    QString folded = text.normalized(QString::NormalizationForm_KD);
    folded.removeIf([](QChar c) {
        return c.category() == QChar::Mark_NonSpacing;
    });
    return folded.toCaseFolded();
}

SearchQuery::SearchQuery(const QString &text)
    : m_words(fold(text).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts))
{
}

bool SearchQuery::isEmpty() const
{
    return m_words.isEmpty();
}

bool SearchQuery::matches(const QStringList &fields) const
{
    const QString text = fold(fields.join(QLatin1Char('\n')));
    return std::ranges::all_of(m_words, [&text](const QString &word) {
        return text.contains(word);
    });
}
