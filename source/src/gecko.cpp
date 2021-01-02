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

bool Gecko::coinPriceChange(const String& coin, const String& currency, double& price, double& price_usd, double& change) const
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

bool Gecko::coinChart(const String& coin, const String& currency, std::vector<double>& prices, double& max, double& min) const
{
    String cleanCoin(cleanUp(coin));
    String cleanCurrency(cleanUp(currency));

    String url("https://api.coingecko.com/api/v3/coins/");
    url += cleanCoin;
    url += "/market_chart?vs_currency=";
    url += cleanCurrency;
    url += "&days=2";

    DynamicJsonDocument filter(16);
    filter["prices"] = true;

    DynamicJsonDocument doc(DYNAMIC_JSON_CHART_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        max = 0;
        min = std::numeric_limits<double>::max();
        JsonArray jPrices = doc["prices"];

        uint16_t ii(jPrices.size());
        for (const auto& p : jPrices) {
            if (ii <= 24) { // only the last 24 values
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
        return prices.size() == 24;
    }
    return false;
}

bool Gecko::coinDetails(const String& coin, String& name, String& symbol) const
{
    String cleanCoin(cleanUp(coin));

    String url("https://api.coingecko.com/api/v3/coins/");
    url += cleanCoin;
    url += "?localization=false&tickers=false&market_data=false&community_data=false&developer_data=false&sparkline=false";

    DynamicJsonDocument filter(32);
    filter["symbol"] = true;
    filter["name"] = true;
    DynamicJsonDocument doc(DYNAMIC_JSON_COIN_DETAILS_SIZE);

    if (m_http.read(url.c_str(), doc, filter)) {
        symbol = doc["symbol"] | "";
        name = doc["name"] | "";
        symbol.toUpperCase();
        return true;
    }
    return false;
}

bool Gecko::isValidCoin(const String& coin) const
{
    String url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += cleanUp(coin);
    url += "&vs_currencies=usd";

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc[cleanUp(coin).c_str()]["usd"] | std::numeric_limits<double>::max();
        return gecko_says != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::isValidCurrency(const String& currency) const
{
    String url("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=");
    url += cleanUp(currency);

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc["bitcoin"][cleanUp(currency).c_str()] | std::numeric_limits<double>::max();
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
