#pragma once
#include <Arduino.h>

class HttpJson;

class Gecko {
public:
    Gecko(HttpJson& http);

    bool coinPriceChange(const std::string& coin, const std::string& currency, double& price, double& change);

    bool isValidCoin(const std::string& coin);
    bool isValidCurrency(const std::string& currency);
    bool ping();

private:
    HttpJson& m_http;
};
