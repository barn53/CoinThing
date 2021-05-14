#include "common.h"
#include "gecko.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define MIN_BRIGHTNESS 10

#define DYNAMIC_JSON_CONFIG_SIZE 1536
#define STATIC_JSON_CONFIG_SIZE 1536

SettingsV12::SettingsV12()
{
}

void SettingsV12::begin()
{
    read();
}

void SettingsV12::set(const char* json)
{
    LOG_FUNC
    {
        StaticJsonDocument<STATIC_JSON_CONFIG_SIZE> doc;
        deserializeJson(doc, json);

        m_mode = static_cast<Mode>(doc["mode"] | static_cast<uint8_t>(Mode::ONE_COIN));

        m_coins.clear();
        for (JsonObject elem : doc["coins"].as<JsonArray>()) {
            Coin c;
            c.id = elem["id"] | "";
            c.symbol = elem["symbol"] | "";
            c.name = elem["name"] | "";
            m_coins.emplace_back(c);
        }

        m_currency = doc["currencies"][0] | "";
        m_currency2 = doc["currencies"][1] | "";

        m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
        m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
        m_swap_interval = static_cast<SwapInterval>(doc["swap_interval"] | static_cast<uint8_t>(SwapInterval::SEC_5));
        m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
        m_heartbeat = doc["heartbeat"] | true;
    }

    trace();
    write();
}

void SettingsV12::read()
{
    LOG_FUNC

    if (SPIFFS.exists(SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(SETTINGS_FILE, "r");
        if (file) {
            SERIAL_PRINTLN("Read settings: " SETTINGS_FILE);
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);

            ReadBufferingStream bufferedFile { file, 64 };
#if COIN_THING_SERIAL > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            deserializeJson(doc, loggingStream);
            Serial.println();
#else
            deserializeJson(doc, bufferedFile);
#endif

            m_mode = static_cast<Mode>(doc["mode"] | static_cast<uint8_t>(Mode::ONE_COIN));

            m_coins.clear();
            for (JsonObject elem : doc["coins"].as<JsonArray>()) {
                Coin c;
                c.id = elem["id"] | "";
                c.symbol = elem["symbol"] | "";
                c.name = elem["name"] | "";
                m_coins.emplace_back(c);
            }

            m_currency = doc["currencies"][0] | "";
            m_currency2 = doc["currencies"][1] | "";

            m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
            m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
            m_swap_interval = static_cast<SwapInterval>(doc["swap_interval"] | static_cast<uint8_t>(SwapInterval::SEC_5));
            m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
            m_heartbeat = doc["heartbeat"] | true;

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
    }
}

void SettingsV12::write() const
{
    LOG_FUNC

    File file = SPIFFS.open(SETTINGS_FILE, "w");
    if (file) {
        file.printf(R"({"mode":%u,)", static_cast<uint8_t>(m_mode));
        file.print(R"("coins":[)");
        bool first(true);
        for (const auto& c : m_coins) {
            if (first) {
                first = false;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"({"id":"%s","symbol":"%s","name":"%s"})", c.id.c_str(), c.symbol.c_str(), c.name.c_str());
        }
        file.print(R"(],)");
        file.printf(R"("currencies":["%s","%s"],)", m_currency.c_str(), m_currency2.c_str());
        file.printf(R"("swap_interval":%u,)", static_cast<uint8_t>(m_swap_interval));
        file.printf(R"("chart_period":%u,)", static_cast<uint8_t>(m_chart_period));
        file.printf(R"("chart_style":%u,)", static_cast<uint8_t>(m_chart_style));
        file.printf(R"("number_format":%u,)", static_cast<uint8_t>(m_number_format));
        file.printf(R"("heartbeat":%s)", m_heartbeat ? "true" : "false");
        file.print(R"(})");
        file.close();
    }
}

void SettingsV12::deleteFile() const
{
    SPIFFS.remove(SETTINGS_FILE);
    SPIFFS.remove(BRIGHTNESS_FILE);
}

bool SettingsV12::valid() const
{
    return SPIFFS.exists(SETTINGS_FILE);
}

void SettingsV12::trace() const
{
#if COIN_THING_SERIAL > 0
    Serial.printf("Mode: >%u<\n", m_mode);
    Serial.printf("Coins:\n");
    for (const auto& c : m_coins) {
        Serial.printf("id: >%s<, name: >%s<, symbol: >%s<, \n", c.id.c_str(), c.name.c_str(), c.symbol.c_str());
    }
    Serial.printf("Currency:       >%s<\n", m_currency.c_str());
    Serial.printf("Currency 2:     >%s<\n", m_currency2.c_str());
    Serial.printf("Number format:  >%u<\n", m_number_format);
    Serial.printf("Chart period:   >%u<\n", m_chart_period);
    Serial.printf("Swap interval:  >%u<\n", m_swap_interval);
    Serial.printf("Chart style:    >%u<\n", m_chart_style);
    Serial.printf("Heart beat:     >%s<\n", (m_heartbeat ? "true" : "false"));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Settings::Settings()
{
}

bool Settings::begin(const Gecko& gecko)
{
    read(gecko);
    write();
    m_last_change = millis_test();

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

void Settings::set(const Gecko& gecko, const char* json)
{

    m_valid = true;
    m_last_change = millis_test();
    write();
}

bool Settings::read(const Gecko& gecko)
{
    //     m_valid = false;
    //     if (SPIFFS.exists(SETTINGS_FILE)) {
    //         File file;
    //         file = SPIFFS.open(SETTINGS_FILE, "r");
    //         if (file) {
    //             Serial.println("open " SETTINGS_FILE);
    //             DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
    //
    // #if COIN_THING_SERIAL > 0
    //             ReadLoggingStream loggingStream(file, Serial);
    //             deserializeJson(doc, loggingStream);
    //             Serial.println();
    // #else
    //             deserializeJson(doc, file);
    // #endif
    //
    //             m_coin = doc["coin"] | "";
    //             m_currency = doc["currency"] | "";
    //             m_currency2 = doc["currency2"] | "usd";
    //             m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    //             m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    //             m_swap_interval = static_cast<SwapInterval>(doc["chart_swap_interval"] | static_cast<uint8_t>(SwapInterval::SEC_5));
    //             m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    //             m_heartbeat = doc["heartbeat"] | true;
    //
    //             m_name = doc["name"] | "";
    //             m_symbol = doc["symbol"] | "";
    //
    //             m_brightness = doc["brightness"] | std::numeric_limits<uint8_t>::max();
    //
    //             if (m_number_format > NumberFormat::DECIMAL_DOT) {
    //                 m_number_format = NumberFormat::DECIMAL_DOT;
    //             }
    //
    //             if (m_chart_period > ChartPeriod::ALL_PERIODS) {
    //                 m_chart_period = ChartPeriod::ALL_PERIODS;
    //             }
    //
    //             if (m_swap_interval > SwapInterval::MIN_5) {
    //                 m_swap_interval = SwapInterval::MIN_5;
    //             }
    //
    //             if (m_chart_style > ChartStyle::HIGH_LOW_FIRST_LAST) {
    //                 m_chart_style = ChartStyle::HIGH_LOW_FIRST_LAST;
    //             }
    //
    //             // Close the file (Curiously, File's destructor doesn't close the file)
    //             file.close();
    //
    //             if (gecko.isValidCoin(m_coin.c_str())) {
    //                 if (gecko.isValidCurrency(m_currency.c_str())) {
    //                     if (gecko.isValidCurrency(m_currency2.c_str())) {
    //                         m_valid = true;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     return m_valid;

    return true;
}

void Settings::write()
{
    //    deleteFile();
    //
    //    if (m_valid) {
    //        File file = SPIFFS.open(SETTINGS_FILE, "w");
    //
    //        if (file) {
    //            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
    //            doc["coin"] = m_coin.c_str();
    //            doc["currency"] = m_currency.c_str();
    //            doc["currency2"] = m_currency2.c_str();
    //            doc["number_format"] = static_cast<uint8_t>(m_number_format);
    //            doc["chart_period"] = static_cast<uint8_t>(m_chart_period);
    //            doc["chart_swap_interval"] = static_cast<uint8_t>(m_swap_interval);
    //            doc["chart_style"] = static_cast<uint8_t>(m_chart_style);
    //            doc["heartbeat"] = m_heartbeat;
    //            doc["name"] = m_name.c_str();
    //            doc["symbol"] = m_symbol.c_str();
    //            doc["brightness"] = m_brightness;
    //
    //            serializeJson(doc, file);
    //            file.close();
    //        }
    //    }
}

void Settings::deleteFile()
{
    //SPIFFS.remove(SETTINGS_FILE);
}
