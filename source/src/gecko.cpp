#include "gecko.h"
#include "http.h"
#include "utils.h"
#include <ArduinoJson.h>

// https://arduinojson.org/v6/assistant/
#define DYNAMIC_JSON_PING_SIZE 64
#define DYNAMIC_JSON_VALID_SIZE 256
#define DYNAMIC_JSON_PRICE_CHANGE_SIZE 256
#define DYNAMIC_JSON_COIN_DETAILS_SIZE 3072
#define DYNAMIC_JSON_CHART_SIZE 3072

Gecko::Gecko(HttpJson& http)
    : m_http(http)
{
}

bool Gecko::coinPriceChange(const char* coin, const char* currency, double& price, double& price_usd, double& change) const
{
    String cleanCoin(cleanUp(coin));
    String cleanCurrency(cleanUp(currency));
    String cleanChange(cleanCurrency);
    cleanChange += "_24h_change";

    String url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += cleanCoin;
    url += "&vs_currencies=usd,";
    url += cleanCurrency;
    url += "&include_24hr_change=true";

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        price = doc[cleanCoin.c_str()][cleanCurrency.c_str()] | std::numeric_limits<double>::max();
        price_usd = doc[cleanCoin.c_str()]["usd"] | std::numeric_limits<double>::max();
        change = doc[cleanCoin.c_str()][cleanChange.c_str()] | std::numeric_limits<double>::max();
        return price != std::numeric_limits<double>::max() && change != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::coinChart(const char* coin, const char* currency, std::vector<double>& prices, double& max, double& min, Settings::Chart chart) const
{
    String cleanCoin(cleanUp(coin));
    String cleanCurrency(cleanUp(currency));

    String url("https://api.coingecko.com/api/v3/coins/");
    url += cleanCoin;
    url += "/market_chart?vs_currency=";
    url += cleanCurrency;
    uint8_t numValues(24);
    if (chart == Settings::Chart::CHART_24_H) {
        url += "&days=2";
    } else if (chart == Settings::Chart::CHART_30_D) {
        url += "&days=30&interval=daily";
        numValues = 30;
    } else {
        // Settings::Chart::CHART_BOTH makes no sense here!
        return false;
    }

    DynamicJsonDocument filter(16);
    filter["prices"] = true;

    DynamicJsonDocument doc(DYNAMIC_JSON_CHART_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        max = 0;
        min = std::numeric_limits<double>::max();
        JsonArray jPrices = doc["prices"];

        uint16_t ii(jPrices.size());
        for (const auto& p : jPrices) {
            if (ii <= numValues) { // trim result
                auto f(p[1].as<double>());
                if (f < min) {
                    min = f;
                }
                if (f > max) {
                    max = f;
                }
                prices.emplace_back(f);
            }
            --ii;
        }
        return prices.size() == numValues;
    }
    return false;
}

bool Gecko::coinDetailsAPI(const char* coin, String& symbol, String& name) const
{
    String url("https://api.coingecko.com/api/v3/coins/");
    url += coin;
    url += "?localization=false&tickers=false&market_data=false&community_data=false&developer_data=false&sparkline=false";

    DynamicJsonDocument filter(32);
    filter["symbol"] = true;
    filter["name"] = true;

    // "sentiment_votes_up_percentage" - todo sentiment lamp

    DynamicJsonDocument doc(DYNAMIC_JSON_COIN_DETAILS_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        symbol = doc["symbol"] | "";
        name = doc["name"] | "";
        symbol.toUpperCase();
        return true;
    }
    return false;
}

bool Gecko::isValidCoinAPI(const char* coin) const
{
    String url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += coin;
    url += "&vs_currencies=usd";

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc[cleanUp(coin).c_str()]["usd"] | std::numeric_limits<double>::max();
        return gecko_says != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::ping() const
{
    DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);

    if (m_http.read("https://api.coingecko.com/api/v3/ping", doc)) {
        const char* gecko_says = doc["gecko_says"] | ""; // "(V3) To the Moon!"
        return strcmp(gecko_says, "(V3) To the Moon!") == 0;
    }
    return false;
}

bool Gecko::coinDetails(const String& coinOrSymbol, String& coin, String& symbol, String& name) const
{
    String cleanCoinOrSymbol(cleanUp(coinOrSymbol));
    String upperCoinOrSymbol(cleanCoinOrSymbol);
    upperCoinOrSymbol.toUpperCase();

    for (const auto& c : coins) {
        if (strcmp(c.id, cleanCoinOrSymbol.c_str()) == 0
            || strcmp(c.symbol, upperCoinOrSymbol.c_str()) == 0) {
            coin = c.id;
            name = c.name;
            symbol = c.symbol;
            return true;
        }
    }
    coin = cleanCoinOrSymbol;
    return coinDetailsAPI(coin.c_str(), symbol, name);
}

bool Gecko::isValidCoin(const char* coinOrSymbol) const
{
    String cleanCoinOrSymbol(cleanUp(coinOrSymbol));
    String upperCoinOrSymbol(cleanCoinOrSymbol);
    upperCoinOrSymbol.toUpperCase();

    for (const auto& c : coins) {
        if (strcmp(c.id, cleanCoinOrSymbol.c_str()) == 0
            || strcmp(c.symbol, upperCoinOrSymbol.c_str()) == 0) {
            return true;
        }
    }

    return isValidCoinAPI(cleanCoinOrSymbol.c_str());
}

bool Gecko::isValidCurrency(const char* currency) const
{
    return isCurrency(cleanUp(currency).c_str());
    // no API fallback for currencies
}
