#include "gecko.h"
#include "http.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#define DYNAMIC_JSON_PING_SIZE 64
#define DYNAMIC_JSON_VALID_SIZE 256
#define DYNAMIC_JSON_PRICE_CHANGE_SIZE 256

namespace {

std::string cleanUp(const std::string& s)
{
    std::string ret(s.substr(0, s.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")));
    return ret;
}

}

Gecko::Gecko(HttpJson& http)
    : m_http(http)
{
}

bool Gecko::coinPriceChange(const std::string& coin, const std::string& currency, double& price, double& change)
{
    std::string cleanCoin(cleanUp(coin));
    std::string cleanCurrency(cleanUp(currency));
    std::string cleanChange(cleanCurrency);
    cleanChange += "_24h_change";

    std::string url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += cleanCoin;
    url += "&vs_currencies=";
    url += cleanCurrency;
    url += "&include_24hr_change=true";

    DynamicJsonDocument doc(DYNAMIC_JSON_PRICE_CHANGE_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        price = doc[cleanCoin.c_str()][cleanCurrency.c_str()] | std::numeric_limits<double>::max();
        change = doc[cleanCoin.c_str()][cleanChange.c_str()] | std::numeric_limits<double>::max();
        return price != std::numeric_limits<double>::max() && change != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::isValidCoin(const std::string& coin)
{
    std::string url("https://api.coingecko.com/api/v3/simple/price?ids=");
    url += cleanUp(coin);
    url += "&vs_currencies=usd";

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc[cleanUp(coin).c_str()]["usd"] | std::numeric_limits<double>::max();
        return gecko_says != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::isValidCurrency(const std::string& currency)
{
    std::string url("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=");
    url += cleanUp(currency);

    DynamicJsonDocument doc(DYNAMIC_JSON_VALID_SIZE);

    if (m_http.read(url.c_str(), doc)) {
        auto gecko_says = doc["bitcoin"][cleanUp(currency).c_str()] | std::numeric_limits<double>::max();
        return gecko_says != std::numeric_limits<double>::max();
    }
    return false;
}

bool Gecko::ping()
{
    DynamicJsonDocument doc(DYNAMIC_JSON_PING_SIZE);

    if (m_http.read("https://api.coingecko.com/api/v3/ping", doc)) {
        const char* gecko_says = doc["gecko_says"] | ""; // "(V3) To the Moon!"
        return strcmp(gecko_says, "(V3) To the Moon!") == 0;
    }
    return false;
}
