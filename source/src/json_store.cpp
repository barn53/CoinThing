#include "json_store.h"

#include <ArduinoJson.h>
#include <StreamUtils.h>

#define FS_NO_GLOBALS
#include <FS.h>
using FS = fs::FS;
using File = fs::File;
using Dir = fs::Dir;

#define JSON_DOCUMENT_STORE_SIZE 512
#define JSON_FILTER_STORE_SIZE 16

JsonStore::JsonStore(const char* file)
    : m_file(file)
{
}

bool JsonStore::get(const char* key, String& value) const
{
    bool ret(false);
    value.clear();

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);
        DynamicJsonDocument filter(JSON_FILTER_STORE_SIZE);
        filter[key] = true;

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
#else
        DeserializationError error = deserializeJson(doc, bufferedFile, DeserializationOption::Filter(filter));
#endif

        if (!error) {
            value = doc[key] | "";
            ret = true;
        }
    }
    file.close();

    return ret;
}

bool JsonStore::set(const char* key, const char* value) const
{
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
#else
        DeserializationError error = deserializeJson(doc, bufferedFile);
#endif

        file.close();
        if (!error) {
            doc[key] = value;

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

bool JsonStore::remove(const char* key) const
{
    bool ret(false);

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream);
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

bool JsonStore::has(const char* key) const
{
    bool ret(false);

    File file = SPIFFS.open(m_file, "r");
    if (file) {
        DynamicJsonDocument doc(JSON_DOCUMENT_STORE_SIZE);
        DynamicJsonDocument filter(JSON_FILTER_STORE_SIZE);
        filter[key] = true;

        ReadBufferingStream bufferedFile { file, 64 };

#if COIN_THING_SERIAL > 0
        ReadLoggingStream loggingStream(bufferedFile, Serial);
        DeserializationError error = deserializeJson(doc, loggingStream, DeserializationOption::Filter(filter));
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
    File file = SPIFFS.open(m_file, "r");
    if (file) {
        return true;
        file.close();
    }
    return false;
}

void JsonStore::del() const
{
    SPIFFS.remove(m_file);
}

void JsonStore::create() const
{
    File file = SPIFFS.open(m_file, "w");
    if (file) {
        file.write("{}");
        file.close();
    }
}
