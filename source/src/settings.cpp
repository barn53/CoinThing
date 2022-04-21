#include "common.h"
#include "gecko.h"
#include "utils.h"
#include <ESP8266WiFi.h>
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
        LOG_I_PRINT(F("deserializeJson() failed: "));
        LOG_I_PRINTLN(error.f_str());
    }
}

void Settings::read()
{
    LOG_FUNC

    if (SPIFFS.exists(SETTINGS_FILE)) {
        File file;
        file = SPIFFS.open(SETTINGS_FILE, "r");
        if (file) {
            LOG_I_PRINTLN("Read settings: " SETTINGS_FILE);
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
                LOG_I_PRINT(F("deserializeJson() failed: "));
                LOG_I_PRINTLN(error.f_str());
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

    m_mode = static_cast<Mode>(doc[F("mode")] | static_cast<uint8_t>(Mode::ONE_COIN));

    m_coins.clear();
    for (JsonObject elem : doc[F("coins")].as<JsonArray>()) {
        Coin c;
        c.id = elem[F("id")] | "";
        c.symbol = elem[F("symbol")] | "";
        c.name = elem[F("name")] | "";
        m_coins.emplace_back(c);
    }

    size_t ii(0);
    for (JsonObject elem : doc[F("currencies")].as<JsonArray>()) {
        Currency c;
        c.currency = elem[F("currency")] | "";
        c.symbol = elem[F("symbol")] | c.currency;
        m_currencies[ii] = c;
        ++ii;
        if (ii >= m_currencies.size()) {
            break;
        }
    }

    m_number_format = static_cast<NumberFormat>(doc[F("number_format")] | static_cast<uint8_t>(NumberFormat::DECIMAL_DOT));
    m_small_decimal_number = static_cast<SmallDecimalNumberFormat>(doc[F("small_decimal_number")] | static_cast<uint8_t>(SmallDecimalNumberFormat::NORMAL));
    m_currency_symbol_position = static_cast<CurrencySymbolPosition>(doc[F("currency_symbol_position")] | static_cast<uint8_t>(CurrencySymbolPosition::LEADING));
    m_chart_period = doc[F("chart_period")] | static_cast<uint8_t>(ChartPeriod::PERIOD_24_H);
    m_swap_interval = static_cast<Swap>(doc[F("swap_interval")] | static_cast<uint8_t>(Swap::INTERVAL_1));
    m_chart_style = static_cast<ChartStyle>(doc[F("chart_style")] | static_cast<uint8_t>(ChartStyle::SIMPLE));
    m_heartbeat = doc[F("heartbeat")] | true;

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
        DynamicJsonDocument doc(JSON_DOCUMENT_CONFIG_SIZE);

        doc[F("mode")] = static_cast<uint8_t>(m_mode);

        JsonArray coins = doc.createNestedArray(F("coins"));
        for (const auto& c : m_coins) {
            JsonObject coin = coins.createNestedObject();
            coin[F("id")] = c.id.c_str();
            coin[F("symbol")] = c.symbol.c_str();
            coin[F("name")] = c.name.c_str();
        }

        JsonArray currencies = doc.createNestedArray("currencies");
        for (const auto& c : m_currencies) {
            JsonObject currency = currencies.createNestedObject();
            currency[F("currency")] = c.currency.c_str();
            currency[F("symbol")] = c.symbol.c_str();
        }

        doc[F("swap_interval")] = static_cast<uint8_t>(m_swap_interval);
        doc[F("chart_period")] = static_cast<uint8_t>(m_chart_period);
        doc[F("chart_style")] = static_cast<uint8_t>(m_chart_style);
        doc[F("number_format")] = static_cast<uint8_t>(m_number_format);
        doc[F("small_decimal_number")] = static_cast<uint8_t>(m_small_decimal_number);
        doc[F("currency_symbol_position")] = static_cast<uint8_t>(m_currency_symbol_position);
        doc[F("heartbeat")] = m_heartbeat;

        serializeJson(doc, file);
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
    LOG_I_PRINTF("Mode: >%u<\n", m_mode)
    LOG_I_PRINTLN("Coins:")
    for (const auto& c : m_coins) {
        LOG_I_PRINTF("id: >%s<, name: >%s<, symbol: >%s<, \n", c.id.c_str(), c.name.c_str(), c.symbol.c_str())
    }
    LOG_I_PRINTF("Currency:                 >%s< >%s<\n", m_currencies[0].currency.c_str(), m_currencies[0].symbol.c_str())
    LOG_I_PRINTF("Currency 2:               >%s< >%s<\n", m_currencies[1].currency.c_str(), m_currencies[1].symbol.c_str())
    LOG_I_PRINTF("Number format:            >%u<\n", m_number_format)
    LOG_I_PRINTF("Small decimal number:     >%u<\n", m_small_decimal_number)
    LOG_I_PRINTF("Currency Symbol position: >%u<\n", m_currency_symbol_position)
    LOG_I_PRINTF("Chart period:             >%u<\n", m_chart_period)
    LOG_I_PRINTF("Swap interval:            >%u<\n", m_swap_interval)
    LOG_I_PRINTF("Chart style:              >%u<\n", m_chart_style)
    LOG_I_PRINTF("Heart beat:               >%s<\n", (m_heartbeat ? "true" : "false"))
#endif
}

uint32_t Settings::numberCoins() const
{
    return m_coins.size();
}

uint32_t Settings::validCoinIndex(uint32_t index) const
{
    if (index >= m_coins.size()) {
        index = 0;
    }
    return index;
}

const String& Settings::coin(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].id;
}
const String& Settings::name(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].name;
}
const String& Settings::symbol(uint32_t index) const
{
    return m_coins[validCoinIndex(index)].symbol;
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
            LOG_I_PRINTLN("Read brightness: " BRIGHTNESS_FILE);
            StaticJsonDocument<JSON_DOCUMENT_BRIGHTNESS_SIZE> doc;
            ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
            ReadLoggingStream loggingStream(bufferedFile, Serial);
            DeserializationError error = deserializeJson(doc, loggingStream);
#else
            DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

            if (!error) {
                m_brightness = doc[F("b")] | std::numeric_limits<uint8_t>::max();
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

void Settings::setColorSet(uint8_t colorSet)
{
    File f = SPIFFS.open(COLOR_SET_FILE, "w");
    f.print(colorSet);
    f.close();
}

uint8_t Settings::getColorSet()
{
    if (SPIFFS.exists(COLOR_SET_FILE)) {
        File f = SPIFFS.open(COLOR_SET_FILE, "r");
        String colorSet(f.readString());
        f.close();
        return colorSet.toInt();
    }
    return 0;
}

void Settings::setFakeGeckoServer(String address)
{
    if (address.isEmpty()) {
        SPIFFS.remove(FAKE_GECKO_SERVER_FILE);
    } else {
        File f = SPIFFS.open(FAKE_GECKO_SERVER_FILE, "w");
        f.print(address);
        f.close();
    }
}

bool Settings::isFakeGeckoServer()
{
    return SPIFFS.exists(FAKE_GECKO_SERVER_FILE);
}

String Settings::getGeckoServer()
{
    if (SPIFFS.exists(FAKE_GECKO_SERVER_FILE)) {
        File f = SPIFFS.open(FAKE_GECKO_SERVER_FILE, "r");
        String fakeGeckoServer(f.readString());
        f.close();
        return fakeGeckoServer;
    }
    return F("https://api.coingecko.com");
}

uint8_t Settings::handlePowerupSequenceForResetBegin()
{
    LOG_FUNC

    uint8_t counter(0);
    if (SPIFFS.exists(POWERUP_SEQUENCE_COUNTER_FILE)) {
        File f = SPIFFS.open(POWERUP_SEQUENCE_COUNTER_FILE, "r");
        counter = f.readString().toInt();
        f.close();
        SPIFFS.remove(POWERUP_SEQUENCE_COUNTER_FILE);
    }
    ++counter;
    File f = SPIFFS.open(POWERUP_SEQUENCE_COUNTER_FILE, "w");
    f.print(counter);
    f.close();
    LOG_I_PRINTF("powerup sequence counter: %u\n", counter);
    return counter;
}

void Settings::handlePowerupSequenceForResetEnd()
{
    SPIFFS.remove(POWERUP_SEQUENCE_COUNTER_FILE);
}

void Settings::handlePowerupSequenceForResetEnd(uint8_t powerupSequenceCounter)
{
    handlePowerupSequenceForResetEnd();

    if (powerupSequenceCounter >= POWERUP_SEQUENCE_COUNT_TO_RESET) {
        deleteFile();
        SPIFFS.remove(WIFI_FILE);
        SPIFFS.remove(FAKE_GECKO_SERVER_FILE);
        // keep COLOR_SET_FILE
        WiFi.disconnect();
        delay(3000);
        ESP.restart();
    }
}
