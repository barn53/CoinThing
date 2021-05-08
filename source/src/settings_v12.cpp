#include "settings_v12.h"
#include "common.h"
#include "gecko.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define MIN_BRIGHTNESS 10
#define USER_CONFIG "/settings.json"
#define STATIC_JSON_CONFIG_SIZE 1024

/*

// https://arduinojson.org/v6/assistant/

// char* input;
// size_t inputLength; (optional)

StaticJsonDocument<1024> doc;

DeserializationError error = deserializeJson(doc, input, inputLength);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

int mode = doc["mode"]; // 3

for (JsonObject elem : doc["coins"].as<JsonArray>()) {

  const char* id = elem["id"]; // "bitcoin", "ethereum", "0x", "dogecoin", "ripple", "tether", "cardano", ...
  const char* symbol = elem["symbol"]; // "btc", "eth", "zrx", "doge", "xrp", "usdt", "ada", "dot", ...
  const char* name = elem["name"]; // "Bitcoin", "Ethereum", "0x", "Dogecoin", "XRP", "Tether", "Cardano", ...

}

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
    read(gecko);
    write();
    m_last_change = millis_test();

    return m_valid;
}

uint8_t SettingsV12::brightness() const
{
    return m_valid
        ? m_brightness
        : std::numeric_limits<uint8_t>::max();
}

bool SettingsV12::setBrightness(uint8_t b)
{
    if (b != m_brightness
        && b >= MIN_BRIGHTNESS
        && b <= std::numeric_limits<uint8_t>::max()) {
        m_brightness = b;
        write();
    }
    return true;
}

void SettingsV12::set(const Gecko& gecko, const char* json)
{

    m_valid = true;
    m_last_change = millis_test();
    write();
}

bool SettingsV12::read(const Gecko& gecko)
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
            m_currency2 = doc["currency2"] | "usd";
            m_number_format = static_cast<NumberFormat>(doc["number_format"] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
            m_chart_period = doc["chart_period"] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
            m_chart_swap_interval = static_cast<ChartSwapInterval>(doc["chart_swap_interval"] | static_cast<uint8_t>(ChartSwapInterval::SEC_5));
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

            if (m_chart_swap_interval > ChartSwapInterval::MIN_5) {
                m_chart_swap_interval = ChartSwapInterval::MIN_5;
            }

            if (m_chart_style > ChartStyle::HIGH_LOW_FIRST_LAST) {
                m_chart_style = ChartStyle::HIGH_LOW_FIRST_LAST;
            }

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();

            if (gecko.isValidCoin(m_coin.c_str())) {
                if (gecko.isValidCurrency(m_currency.c_str())) {
                    if (gecko.isValidCurrency(m_currency2.c_str())) {
                        m_valid = true;
                    }
                }
            }
        }
    }
    return m_valid;
}

void SettingsV12::write()
{
    deleteFile();

    if (m_valid) {
        File file = SPIFFS.open(USER_CONFIG, "w");

        if (file) {
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
            doc["coin"] = m_coin.c_str();
            doc["currency"] = m_currency.c_str();
            doc["currency2"] = m_currency2.c_str();
            doc["number_format"] = static_cast<uint8_t>(m_number_format);
            doc["chart_period"] = static_cast<uint8_t>(m_chart_period);
            doc["chart_swap_interval"] = static_cast<uint8_t>(m_chart_swap_interval);
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

void SettingsV12::deleteFile()
{
    SPIFFS.remove(USER_CONFIG);
}
