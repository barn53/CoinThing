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

#define MIN_BRIGHTNESS 10

Settings::Settings()
{
}

bool Settings::begin(const Gecko& gecko)
{
    read(gecko);
    write();
    m_lastChange = millis_test();

    return m_valid;
}

uint8_t Settings::brightness() const
{
    return m_valid
        ? m_brightness
        : std::numeric_limits<uint8_t>::max();
}

bool Settings::setBrightness(uint8_t b)
{
    if (b != m_brightness
        && b >= MIN_BRIGHTNESS
        && b <= std::numeric_limits<uint8_t>::max()) {
        m_brightness = b;
        write();
    }
    return true;
}

Settings::Status Settings::set(const Gecko& gecko, const char* coin, const char* currency, uint8_t number_format, uint8_t chart_period, uint8_t chart_style, bool heartbeat)
{
    Status ret = Status::OK;
    String cleanCoin(cleanUp(coin));
    String cleanCurrency(cleanUp(currency));

    if (gecko.coinDetails(cleanCoin.c_str(), m_coin, m_symbol, m_name)) {
        if (gecko.isValidCurrency(cleanCurrency.c_str())) {
            m_currency = cleanCurrency;

            if (number_format > static_cast<uint8_t>(NumberFormat::DECIMAL_DOT)) {
                number_format = static_cast<uint8_t>(NumberFormat::DECIMAL_DOT);
            }
            m_number_format = static_cast<NumberFormat>(number_format);

            if (chart_period > ChartPeriod::ALL_PERIODS) {
                chart_period = ChartPeriod::ALL_PERIODS;
            }
            m_chart_period = chart_period;

            if (chart_style > static_cast<uint8_t>(ChartStyle::HIGH_LOW_FIRST_LAST)) {
                chart_style = static_cast<uint8_t>(ChartStyle::HIGH_LOW_FIRST_LAST);
            }
            m_chart_style = static_cast<ChartStyle>(chart_style);

            m_heartbeat = heartbeat;

            m_valid = true;
            m_lastChange = millis_test();
            write();
        } else {
            ret = Status::CURRENCY_INVALID;
        }
    } else {
        ret = Status::COIN_INVALID;
    }

    return ret;
}

bool Settings::read(const Gecko& gecko)
{
    m_valid = false;
    if (SPIFFS.exists(USER_CONFIG)) {
        File file;
        file = SPIFFS.open(USER_CONFIG, "r");
        if (file) {
            Serial.println("open " USER_CONFIG);
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);

#if COIN_THING_SERIAL > 0
            ReadLoggingStream loggingStream(file, Serial);
            deserializeJson(doc, loggingStream);
            Serial.println();
#else
            deserializeJson(doc, file);
#endif

            m_coin = doc["coin"] | "";
            m_currency = doc["currency"] | "";
            m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
            m_chart_period = static_cast<ChartPeriod>(doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H));
            m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
            m_heartbeat = doc["heartbeat"] | true;

            m_name = doc["name"] | "";
            m_symbol = doc["symbol"] | "";

            m_brightness = doc["brightness"] | std::numeric_limits<uint8_t>::max();

            if (m_number_format > NumberFormat::DECIMAL_DOT) {
                m_number_format = NumberFormat::DECIMAL_DOT;
            }

            if (m_chart_period > ChartPeriod::ALL_PERIODS) {
                m_chart_period = ChartPeriod::ALL_PERIODS;
            }

            if (m_chart_style > ChartStyle::HIGH_LOW_FIRST_LAST) {
                m_chart_style = ChartStyle::HIGH_LOW_FIRST_LAST;
            }

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();

            if (gecko.isValidCoin(m_coin.c_str())) {
                if (gecko.isValidCurrency(m_currency.c_str())) {
                    m_valid = true;
                }
            }
        }
    }
    return m_valid;
}

void Settings::write()
{
    deleteFile();

    if (m_valid) {
        File file = SPIFFS.open(USER_CONFIG, "w");

        if (file) {
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
            doc["coin"] = m_coin.c_str();
            doc["currency"] = m_currency.c_str();
            doc["number_format"] = static_cast<uint8_t>(m_number_format);
            doc["chart_period"] = static_cast<uint8_t>(m_chart_period);
            doc["chart_style"] = static_cast<uint8_t>(m_chart_style);
            doc["heartbeat"] = m_heartbeat;
            doc["name"] = m_name.c_str();
            doc["symbol"] = m_symbol.c_str();
            doc["brightness"] = m_brightness;

            serializeJson(doc, file);
            file.close();
        }
    }
}

void Settings::deleteFile()
{
    SPIFFS.remove(USER_CONFIG);
}
