#include "settings.h"
#include "gecko.h"
#include "utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

using fs::File;

#define USER_CONFIG "/config.json"
#define DYNAMIC_JSON_CONFIG_SIZE 256

Settings::Settings(Gecko& gecko)
    : m_gecko(gecko)
{
    SPIFFS.begin();
}

bool Settings::begin()
{
    read();
    write();

    return m_valid;
}

Settings::Status Settings::set(const char* coin, const char* currency, uint8_t number_format)
{
    bool valid(false);
    Status ret = Status::OK;

    if (m_gecko.isValidCoin(cleanUp(coin))) {
        if (m_gecko.isValidCurrency(cleanUp(currency))) {
            m_gecko.coinDetails(coin, m_name, m_symbol);
            valid = true;
        } else {
            ret = Status::CURRENCY_INVALID;
        }
    } else {
        ret = Status::COIN_INVALID;
    }

    if (valid) {
        m_coin = cleanUp(coin);
        m_currency = cleanUp(currency);
        m_number_format = static_cast<NumberFormat>(number_format);
        if (m_number_format > NumberFormat::DECIMAL_DOT) {
            m_number_format = NumberFormat::DECIMAL_DOT;
        }
        m_valid = true;
        m_displayed = false;
        write();
    }

    return ret;
}

bool Settings::read()
{
    m_valid = false;
    if (SPIFFS.exists(USER_CONFIG)) {
        File file;
        file = SPIFFS.open(USER_CONFIG, "r");
        if (file) {
            Serial.println("open " USER_CONFIG);
            ReadLoggingStream loggingStream(file, Serial);
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
            deserializeJson(doc, loggingStream);
            Serial.println();

            m_coin = doc["coin"] | "";
            m_currency = doc["currency"] | "";
            m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
            if (m_number_format > NumberFormat::DECIMAL_DOT) {
                m_number_format = NumberFormat::DECIMAL_DOT;
            }
            m_name = doc["name"] | "";
            m_symbol = doc["symbol"] | "";

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();

            if (m_gecko.isValidCoin(m_coin)) {
                if (m_gecko.isValidCurrency(m_currency)) {
                    m_valid = true;
                    m_displayed = false;
                }
            }
        }
    }
    return m_valid;
}

void Settings::write() const
{
    SPIFFS.remove(USER_CONFIG);

    if (m_valid) {
        File file = SPIFFS.open(USER_CONFIG, "w");

        if (file) {
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
            doc["coin"] = m_coin.c_str();
            doc["currency"] = m_currency.c_str();
            doc["number_format"] = static_cast<uint8_t>(m_number_format);
            doc["name"] = m_name.c_str();
            doc["symbol"] = m_symbol.c_str();

            serializeJson(doc, file);
            file.close();
        }
    }
}
