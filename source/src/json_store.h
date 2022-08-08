#pragma once
#include <Arduino.h>

class JsonStore {
public:
    JsonStore(const String& file);

    bool get(const String& key, String& value) const;
    bool set(const String& key, const String& value) const;
    bool remove(const String& key) const;
    bool has(const String& key) const;
    bool exists() const;
    void deleteStore() const;

private:
    void create() const;
    String m_file;
};
