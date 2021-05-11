#include "common.h"
#include "gecko.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define MIN_BRIGHTNESS 10

#define DYNAMIC_JSON_CONFIG_SIZE 1024 // todo: remove
#define STATIC_JSON_CONFIG_SIZE 1024

/*

// https://arduinojson.org/v6/assistant/

// char* input;
// size_t inputLength; (optional)

StaticJsonDocument<384> doc;

DeserializationError error = deserializeJson(doc, input, inputLength);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

int mode = doc["mode"]; // 3

JsonArray coins = doc["coins"];
const char* coins_0 = coins[0]; // "bitcoin"
const char* coins_1 = coins[1]; // "ethereum"
const char* coins_2 = coins[2]; // "0x"
const char* coins_3 = coins[3]; // "dogecoin"
const char* coins_4 = coins[4]; // "ripple"
const char* coins_5 = coins[5]; // "tether"
const char* coins_6 = coins[6]; // "cardano"
const char* coins_7 = coins[7]; // "polkadot"
const char* coins_8 = coins[8]; // "terra-luna"
const char* coins_9 = coins[9]; // "pancakeswap-token"

const char* currencies_0 = doc["currencies"][0]; // "eur"
const char* currencies_1 = doc["currencies"][1]; // "usd"

int swap_interval = doc["swap_interval"]; // 2
int chart_period = doc["chart_period"]; // 4
int chart_style = doc["chart_style"]; // 2
int number_format = doc["number_format"]; // 1
bool heartbeat = doc["heartbeat"]; // true

*/

SettingsV12::SettingsV12()
{
}

bool SettingsV12::begin(const Gecko& gecko)
{
    return true;
}

void SettingsV12::set(const Gecko& gecko, const char* json, String& error)
{
    LOG_FUNC

    Coin coin;

    Serial.printf("open /coinsa\n");
    File f = SPIFFS.open("/coinsa", "r");

    size_t lines(0);
    char line[200];
    while (f.available()) {
        f.readBytesUntil('\n', line, sizeof(line));
        if (strstr(line, "SingularityNET") != nullptr) {
            break;
        }
        ++lines;
    }
    Serial.printf("/coinsa #lines: %u\n", lines);
    f.close();

    return;

    m_valid = true;
    error.clear();

    StaticJsonDocument<STATIC_JSON_CONFIG_SIZE> doc;
    DeserializationError jsonError = deserializeJson(doc, json);
    if (jsonError) {
        error = jsonError.f_str();
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error);
        return;
    }

    m_mode = static_cast<Mode>(doc["mode"] | static_cast<uint8_t>(Mode::ONE_COIN));

    m_coins.clear();
    JsonArray coins = doc["coins"];
    for (const char* c : coins) {
        Coin coin;
        if (gecko.coinDetails(c, coin.id, coin.symbol, coin.name)) {
            m_coins.emplace_back(coin);
        } else {
            m_valid = false;
            error += c;
        }
    }

    m_currency = doc["currencies"][0] | "";
    m_currency2 = doc["currencies"][1] | "usd";

    m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    m_swap_interval = static_cast<SwapInterval>(doc["swap_interval"] | static_cast<uint8_t>(SwapInterval::SEC_5));
    m_chart_style = static_cast<ChartStyle>(doc["chart_style"] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    m_heartbeat = doc["heartbeat"] | true;

    trace();
    SERIAL_PRINTLN(error)

    if (!m_valid) {
        read(gecko);
    }
}

bool SettingsV12::read(const Gecko& gecko)
{
    return true;
}
void SettingsV12::write() const
{
}
void SettingsV12::deleteFile() const
{
}

void SettingsV12::trace() const
{
#if COIN_THING_SERIAL > 0
    Serial.printf("Coins:\n");
    for (const auto& c : m_coins) {
        Serial.printf("id: >%s<, name: >%s<, symbol: >%s<, \n", c.id.c_str(), c.name.c_str(), c.symbol.c_str());
    }
    Serial.printf("Currency:                >%s<\n", m_currency.c_str());
    Serial.printf("Currency 2:              >%s<\n", m_currency2.c_str());
    Serial.printf("Settings number format:  >%u<\n", m_number_format);
    Serial.printf("Settings chart period:   >%u<\n", m_chart_period);
    Serial.printf("Settings swap interval:  >%u<\n", m_swap_interval);
    Serial.printf("Settings chart style:    >%u<\n", m_chart_style);
    Serial.printf("Settings heart beat:     >%s<\n", (m_heartbeat ? "true" : "false"));
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
