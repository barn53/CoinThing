#pragma once
#include <Arduino.h>

class HttpJson;

class Gecko {
public:
    Gecko(HttpJson& http);

    bool coinPriceChange(const String& coin, const String& currency, double& price, double& price_usd, double& change) const;
    bool coinChart(const String& coin, const String& currency, std::vector<double>& prices, double& max, double& min) const;
    bool coinDetails(const String& coin, String& name, String& symbol) const;

    bool isValidCoin(const String& coin) const;
    bool isValidCurrency(const String& currency) const;
    bool ping() const;

private:
    HttpJson& m_http;
};
