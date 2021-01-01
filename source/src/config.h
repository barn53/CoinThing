#pragma once
#include <Arduino.h>

class Gecko;

class Config {
public:
    Config(Gecko& http);

    bool begin();
    void read();
    void write();
    const std::string& coin() const { return m_coin; }
    const std::string& currency() const { return m_currency; }
    bool valid() const { return m_valid; }

    Gecko& getGecko() { return m_gecko; }

private:
    std::string m_coin;
    std::string m_currency;
    bool m_valid { false };

    Gecko& m_gecko;
};
