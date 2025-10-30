// Licensed under the EUPL
#pragma once

#include <KRunner/AbstractRunner>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QDateTime>
#include <QHash>
#include <QSet>
#include <QRegularExpression>
#include <QString>
#include <QDir>

class Currencies final : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    Currencies(QObject *parent, const KPluginMetaData &data);

    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
    void reloadConfiguration() override;
protected:
    void init() override;

private Q_SLOTS:
    void onExchangeRatesReceive(QNetworkReply *response);

private:
    void checkDateAndUpdateExchangeRates();

    QDateTime lastUpdated;
    QNetworkAccessManager *manager = nullptr;
    QSettings *settings = nullptr;

    QString mainEndpoint = QStringLiteral("https://open.er-api.com/v6/latest/EUR");
    QString mainEndpointShort = QStringLiteral("er-api");

    QString frankfurterEndpoint = QStringLiteral("https://api.frankfurter.dev/v1/latest?base=EUR");
    QString frankfurterEndpointShort = QStringLiteral("frankfurter");

    QRegularExpression queryRegex{QStringLiteral(R"((?<num>\d+)\s(?<scur>[a-zA-Z]{3})\s[iI][nN]\s(?<dcur>[a-zA-Z]{3}))")};

    QHash<QString, double> exchangeRatesForEuro;
    QSet<QString> frankfurterCurrencies;

    bool ignoreFrankfurterCurrencies = false;
    bool useCaching = false;

    QDir CacheDir;
};
