/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <KRunner/AbstractRunner>

class Currencies : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    Currencies(QObject *parent, const KPluginMetaData &data);

    // KRunner::AbstractRunner API
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
};
