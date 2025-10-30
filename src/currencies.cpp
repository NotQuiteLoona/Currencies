// Licensed under the EUPL
#include "currencies.hpp"

#include <KLocalizedString>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QGuiApplication>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QFile>
#include <KConfigGroup>

Currencies::Currencies(QObject *parent, const KPluginMetaData &data)
: KRunner::AbstractRunner(parent, data)
, manager(new QNetworkAccessManager(this))
{
    addSyntax(QStringLiteral("<number> <source currency> in <destination currency>"),
              i18n("Takes a <number> of <source currency> and converts it to <destination currency>."));
}

bool mainCompleted, frankfurterCompleted, initCompleted;

void Currencies::init()
{
    auto env = qEnvironmentVariable("XDG_CONFIG_HOME");
    // https://specifications.freedesktop.org/basedir/latest/#variables
    if (env.isEmpty()) {
        env = QStringLiteral("%1/.config/currencies").arg(qEnvironmentVariable("HOME"));
    }

    CacheDir = QDir(env);

    reloadConfiguration();

    if (!ignoreFrankfurterCurrencies)
    {
        frankfurterCompleted = true;
    }

    connect(manager, &QNetworkAccessManager::finished, this, &Currencies::onExchangeRatesReceive);
    checkDateAndUpdateExchangeRates();
}

void Currencies::checkDateAndUpdateExchangeRates()
{
    const auto currentTime = QDateTime::currentDateTime();

    if (!initCompleted || lastUpdated.daysTo(currentTime) >= 1) {
        const auto mainApiRequest = QNetworkRequest(QUrl(mainEndpoint));
        manager->get(mainApiRequest);

        if (ignoreFrankfurterCurrencies) {
            const auto frankfurterRequest = QNetworkRequest(QUrl(frankfurterEndpoint));
            manager->get(frankfurterRequest);
        }

        lastUpdated = currentTime;
    }
}

void Currencies::onExchangeRatesReceive(QNetworkReply *response)
{
    const QString url = response->url().toString();
    const bool isMainApi = url.contains(mainEndpointShort);
    const QString cacheKey = isMainApi ? mainEndpointShort : frankfurterEndpointShort;

    QByteArray data;
    if (response->error() == QNetworkReply::NoError) {
        data = response->readAll();
    } else {
        if (useCaching) {
            // makes this code very hard to read
            // ReSharper disable once CppTooWideScopeInitStatement
            QFile cache(CacheDir.absoluteFilePath(QStringLiteral("%1.json").arg(cacheKey)));
            if (cache.open(QIODevice::ExistingOnly | QIODevice::ReadOnly)) {
                data = cache.readAll();
                cache.close();
            } else {
                response->deleteLater();
                return;
            }
        } else {
            response->deleteLater();
            return;
        }
    }

    const auto json = QJsonDocument::fromJson(data);
    if (json.isNull()) {
        response->deleteLater();
        return;
    }

    auto root = json.object();
    auto rates = root[QStringLiteral("rates")].toObject();

    if (!isMainApi) {
        frankfurterCurrencies.clear();
    }

    for (auto it = rates.begin(); it != rates.end(); ++it) {
        const QString currency = it.key();

        if (isMainApi) {
            const double exchange = it.value().toDouble();
            exchangeRatesForEuro[currency] = exchange;
        } else {
            frankfurterCurrencies.insert(currency);
        }
    }

    // frankfurter API doesn't include base currency in rates, unlike er-api
    if (!isMainApi) {
        frankfurterCurrencies.insert(QStringLiteral("EUR"));
    }

    if (useCaching) {
        // probably DRY violation, but I don't want to create another method
        QFile cache(CacheDir.absoluteFilePath(QStringLiteral("%1.json").arg(cacheKey)));
        if (cache.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            cache.write(data);
            cache.flush();
            cache.close();
        }

        QFile notification(CacheDir.absoluteFilePath(QStringLiteral("README.txt")));
        if (notification.open(QIODevice::NewOnly | QIODevice::WriteOnly)) {
            notification.write(QStringLiteral("This folder contains cache files for the Currencies KRunner plugin.").toUtf8());
            notification.flush();
            notification.close();
        }
    }

    response->deleteLater();

    if (cacheKey == mainEndpointShort) {
        mainCompleted = true;
    } else {
        frankfurterCompleted = true;
    }

    if (mainCompleted && frankfurterCompleted) {
        initCompleted = true;
    }
}

void Currencies::match(KRunner::RunnerContext &context)
{
    checkDateAndUpdateExchangeRates();

    const auto query = context.query();
    const auto match = queryRegex.match(query);

    if (!match.hasMatch()) {
        return;
    }

    const auto amount = match.captured(QStringLiteral("num")).toDouble();
    const auto sourceCurrency = match.captured(QStringLiteral("scur")).toUpper();
    const auto destinationCurrency = match.captured(QStringLiteral("dcur")).toUpper();

    if (ignoreFrankfurterCurrencies) {
        if (frankfurterCurrencies.contains(sourceCurrency) &&
            frankfurterCurrencies.contains(destinationCurrency)) {
            return;
            }
    }

    // those flags are negating the convention of boolean variables meaning whether you can do something and not whether you can't do it for an ease of read
    const auto dontHaveSourceCurrency = !exchangeRatesForEuro.contains(sourceCurrency);
    // for consistence
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dontHaveDestinationCurrency = !exchangeRatesForEuro.contains(destinationCurrency);

    if (dontHaveSourceCurrency || dontHaveDestinationCurrency) {
        KRunner::QueryMatch m(this);
        m.setId(QStringLiteral("occ-ignore-no-match"));

        QString message;

        if (dontHaveSourceCurrency && dontHaveDestinationCurrency) {
            message = QStringLiteral("Currencies %1 and %2 are").arg(sourceCurrency, destinationCurrency);
        } else {
            message = QStringLiteral("Currency %1 is").arg(dontHaveSourceCurrency ? sourceCurrency : destinationCurrency);
        }

        m.setText(QStringLiteral("%1 not recognized.").arg(message));
        context.addMatch(m);
        return;
    }

    const double sourceRate = exchangeRatesForEuro[sourceCurrency];
    const double destRate = exchangeRatesForEuro[destinationCurrency];
    const double sourceInEur = amount / sourceRate;
    const double result = sourceInEur * destRate;

    KRunner::QueryMatch qMatch(this);

    const QString resultText = QString::number(result, 'f', 2);
    qMatch.setText(resultText);
    qMatch.setIconName(QStringLiteral("accessories-calculator"));
    qMatch.setData(result);
    qMatch.setRelevance(1.0);

    context.addMatch(qMatch);
}

void Currencies::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context)

    // in ideal "occ-ignore" should be in a separate variable for easier change, but it will definitely be an overengineering right now
    if (match.id().startsWith(QStringLiteral("occ-ignore"))) return;

    const auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(QString::number(match.data().toDouble(), 'f', 2));
}

void Currencies::reloadConfiguration()
{
    const KConfigGroup c = config();

    const QString frankfurter = c.readEntry("ignoreFrankfurter", QStringLiteral("true"));
    const QString loadFromCache = c.readEntry("caching", QStringLiteral("true"));

    ignoreFrankfurterCurrencies = (frankfurter == QStringLiteral("true"));
    useCaching = (loadFromCache == QStringLiteral("true"));
}

K_PLUGIN_CLASS_WITH_JSON(Currencies, "currencies.json")

#include "currencies.moc"
#include "moc_currencies.cpp"
