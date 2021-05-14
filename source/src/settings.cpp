#include "common.h"
#include "gecko.h"
#include "utils.h"
#include <StreamUtils.h>

#define MIN_BRIGHTNESS 10

#define JSON_DOCUMENT_CONFIG_SIZE 1536
#define JSON_DOCUMENT_BRIGHTNESS_SIZE 32

Settings::Settings()
{
}

void Settings::begin()
{
    read();
    readBrightness();
}

void Settings::set(const char* json)
{
    LOG_FUNC

    DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (!error) {
        set(doc, true);
    } else {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
}

void Settings::read()
{
    LOG_FUNC

    if (SPIFFS.exists(SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(SETTINGS_FILE, "r");
        if (file) {
            SERIAL_PRINTLN("Read settings: " SETTINGS_FILE);
            DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);

            ReadBufferingStream bufferedFile { file, 64 };
#if COIN_THING_SERIAL > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif
            if (!error) {
                set(doc, false);
            } else {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
            }
            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();
        }
    }
}

void Settings::set(DynamicJsonDocument& doc, bool toFile)
{
    LOG_FUNC

    m_last_change = millis_test();

    m_mode = static_cast<Mode>(doc["mode"] | static_cast<uint8_t>(Mode::ONE_COIN));

    m_coins.clear();
    for (JsonObject elem : doc["coins"].as<JsonArray>()) {
        Coin c;
        c.id = elem["id"] | "";
        c.symbol = elem["symbol"] | "";
        c.name = elem["name"] | "";
        m_coins.emplace_back(c);
    }

    size_t ii(0);
    for (JsonObject elem : doc["currencies"].as<JsonArray>()) {
        Currency c;
        c.currency = elem["currency"] | "";
        c.symbol = elem["symbol"] | c.currency;
        m_currencies[ii] = c;
        ++ii;
        if (ii >= m_currencies.size()) {
            break;
        }
    }

    m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    m_swap_interval = static_cast<SwapInterval>(doc["swap_interval"] | static_cast<uint8_t>(SwapInterval::SEC_5));
    m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    m_heartbeat = doc["heartbeat"] | true;

    trace();
    if (toFile) {
        write();
    }
}

void Settings::write() const
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

        file.print(R"("currencies":[)");
        first = true;
        for (const auto& c : m_currencies) {
            if (first) {
                first = false;
            } else {
                file.print(R"(,)");
            }
            file.printf(R"({"currency":"%s","symbol":"%s"})", c.currency.c_str(), c.symbol.c_str());
        }
        file.print(R"(],)");
        file.printf(R"("swap_interval":%u,)", static_cast<uint8_t>(m_swap_interval));
        file.printf(R"("chart_period":%u,)", static_cast<uint8_t>(m_chart_period));
        file.printf(R"("chart_style":%u,)", static_cast<uint8_t>(m_chart_style));
        file.printf(R"("number_format":%u,)", static_cast<uint8_t>(m_number_format));
        file.printf(R"("heartbeat":%s)", m_heartbeat ? "true" : "false");
        file.print(R"(})");
        file.close();
    }
}

void Settings::deleteFile() const
{
    SPIFFS.remove(SETTINGS_FILE);
    SPIFFS.remove(BRIGHTNESS_FILE);
}

bool Settings::valid() const
{
    return SPIFFS.exists(SETTINGS_FILE);
}

void Settings::trace() const
{
#if COIN_THING_SERIAL > 0
    Serial.printf("Mode: >%u<\n", m_mode);
    Serial.printf("Coins:\n");
    for (const auto& c : m_coins) {
        Serial.printf("id: >%s<, name: >%s<, symbol: >%s<, \n", c.id.c_str(), c.name.c_str(), c.symbol.c_str());
    }
    Serial.printf("Currency:       >%s< >%s<\n", m_currencies[0].currency.c_str(), m_currencies[0].symbol.c_str());
    Serial.printf("Currency 2:     >%s< >%s<\n", m_currencies[1].currency.c_str(), m_currencies[1].symbol.c_str());
    Serial.printf("Number format:  >%u<\n", m_number_format);
    Serial.printf("Chart period:   >%u<\n", m_chart_period);
    Serial.printf("Swap interval:  >%u<\n", m_swap_interval);
    Serial.printf("Chart style:    >%u<\n", m_chart_style);
    Serial.printf("Heart beat:     >%s<\n", (m_heartbeat ? "true" : "false"));
#endif
}

const String& Settings::coin() const
{
    return m_coins[0].id;
}
const String& Settings::name() const
{
    return m_coins[0].name;
}
const String& Settings::symbol() const
{
    return m_coins[0].symbol;
}

const String& Settings::currency() const
{
    return m_currencies[0].currency;
}
const String& Settings::currencySymbol() const
{
    return m_currencies[0].symbol;
}

const String& Settings::currency2() const
{
    return m_currencies[1].currency;
}
const String& Settings::currency2Symbol() const
{
    return m_currencies[1].symbol;
}

uint8_t Settings::brightness() const
{
    return m_brightness;
}

void Settings::readBrightness()
{
    LOG_FUNC

    if (SPIFFS.exists(BRIGHTNESS_FILE)) {
        File file;
        file = SPIFFS.open(BRIGHTNESS_FILE, "r");
        if (file) {
            SERIAL_PRINTLN("Read brightness: " BRIGHTNESS_FILE);
            StaticJsonDocument<JSON_DOCUMENT_BRIGHTNESS_SIZE> doc;

            ReadBufferingStream bufferedFile { file, 64 };
#if COIN_THING_SERIAL > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif
            if (!error) {
                m_brightness = doc["b"] | std::numeric_limits<uint8_t>::max();
            } else {
                m_brightness = std::numeric_limits<uint8_t>::max();
            }

            file.close();
        }
    }
}

void Settings::setBrightness(uint8_t b)
{
    LOG_FUNC

    if (b >= MIN_BRIGHTNESS
        && b <= std::numeric_limits<uint8_t>::max()) {
        m_brightness = b;
        File file = SPIFFS.open(BRIGHTNESS_FILE, "w");
        if (file) {
            file.printf(R"({"b":%u})", m_brightness);
            file.close();
        }
    }
}
