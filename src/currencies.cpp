/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "currencies.h"

#include <KLocalizedString>

Currencies::Currencies(QObject *parent, const KPluginMetaData &data)
    : KRunner::AbstractRunner(parent, data)
{
    // Provide usage help for this plugin
    addSyntax(QStringLiteral("sometriggerword :q:"), i18n("Description for this syntax"));
}

void Currencies::match(KRunner::RunnerContext &context)
{
    const QString term = context.query();
    if (term.compare(QLatin1String("hello"), Qt::CaseInsensitive) == 0) {
        KRunner::QueryMatch match(this);
        match.setText(i18n("Hello from Currencies"));
        context.addMatch(match);
    }
}

void Currencies::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context)
    Q_UNUSED(match)

    // TODO
}

K_PLUGIN_CLASS_WITH_JSON(Currencies, "currencies.json")

// needed for the QObject subclass declared as part of K_PLUGIN_CLASS_WITH_JSON
#include "currencies.moc"

#include "moc_currencies.cpp"
