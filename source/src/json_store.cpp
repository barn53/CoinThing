#include "json_store.h"
#include "utils.h"

#include <ArduinoJson.h>
#include <StreamUtils.h>

#define FS_NO_GLOBALS
#include <FS.h>
using FS = fs::FS;
using File = fs::File;
using Dir = fs::Dir;

#define JSON_DOCUMENT_STORE_SIZE 512
#define JSON_FILTER_STORE_SIZE 16

JsonStore::JsonStore(const String& file)
    : m_file(file)
{
    LOG_FUNC
    LOG_I_PRINTF("file: %s\n", file.c_str());
}

bool JsonStore::get(const String& key, String& value) const
{
    LOG_FUNC
    LOG_I_PRINTF("key:   %s\n", key.c_str());

    bool ret(false);
    value.clear();

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);
        DynamicJsonDocument filter(JSON_FILTER_STORE_SIZE);
        filter[key.c_str()] = true;

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
        LOG_PRINTLN("")
#else
        DeserializationError error = deserializeJson(doc, bufferedFile, DeserializationOption::Filter(filter));
#endif

        if (!error) {
            value = doc[key.c_str()] | "";
            LOG_I_PRINTF("value: %s\n", value.c_str());
            ret = true;
        }
    }
    file.close();

    return ret;
}

bool JsonStore::set(const String& key, const String& value) const
{
    LOG_FUNC
    LOG_I_PRINTF("key:   %s\n", key.c_str());
    LOG_I_PRINTF("value: %s\n", value.c_str());

    bool ret(false);

    File file = SPIFFS.open(m_file, "r");
    if (!file) {
        create();
        file = SPIFFS.open(m_file, "r");
    }

    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream);
        LOG_PRINTLN("")
#else
        DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

        file.close();
        if (!error) {
            doc[key.c_str()] = value;

            file = SPIFFS.open(m_file, "w");
            if (file) {
                serializeJson(doc, file);
                ret = true;
            }
            file.close();
        }
    }

    return ret;
}

bool JsonStore::remove(const String& key) const
{
    LOG_FUNC
    LOG_I_PRINTF("key: %s\n", key.c_str());

    bool ret(false);

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream);
        LOG_PRINTLN("")
#else
        DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

        file.close();
        if (!error) {
            doc.remove(key);

            file = SPIFFS.open(m_file, "w");
            if (file) {
                serializeJson(doc, file);
                ret = true;
            }
            file.close();
        }
    }

    return ret;
}

bool JsonStore::has(const String& key) const
{
    LOG_FUNC
    LOG_I_PRINTF("key: %s\n", key.c_str());

    bool ret(false);

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);
        DynamicJsonDocument filter(JSON_FILTER_STORE_SIZE);
        filter[key.c_str()] = true;

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
        LOG_PRINTLN("")
#else
        DeserializationError error = deserializeJson(doc, bufferedFile, DeserializationOption::Filter(filter));
#endif

        if (!error) {
            ret = doc.containsKey(key);
        }
    }
    file.close();

    return ret;
}

bool JsonStore::exists() const
{
    LOG_FUNC

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        return true;
        file.close();
    }
    return false;
}

void JsonStore::deleteStore() const
{
    LOG_FUNC

    SPIFFS.remove(m_file);
}

void JsonStore::create() const
{
    LOG_FUNC

    File file = SPIFFS.open(m_file, "w");
    if (file) {
        file.write("{}");
        file.close();
    }
}
