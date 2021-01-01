#include "config.h"
#include "gecko.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

using fs::File;

#define USER_CONFIG "/config.json"
#define DYNAMIC_JSON_CONFIG_SIZE 96

Config::Config(Gecko& gecko)
    : m_gecko(gecko)
{
    SPIFFS.begin();
}

bool Config::begin()
{
    read();
    write();

    return m_valid;
}

void Config::read()
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

            // Close the file (Curiously, File's destructor doesn't close the file)
            file.close();

            if (m_gecko.isValidCoin(m_coin)) {
                if (m_gecko.isValidCurrency(m_currency)) {
                    m_valid = true;
                }
            }
        }
    }
}

void Config::write()
{
    SPIFFS.remove(USER_CONFIG);

    if (m_valid) {
        File file = SPIFFS.open(USER_CONFIG, "w");

        if (file) {
            DynamicJsonDocument doc(DYNAMIC_JSON_CONFIG_SIZE);
            doc["coin"] = m_coin.c_str();
            doc["currency"] = m_currency.c_str();

            serializeJson(doc, file);
            file.close();
        }
    }
}
