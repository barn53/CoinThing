#pragma once
#include <Arduino.h>

class JsonStore {
public:
    JsonStore(const char* file);

    bool get(const char* key, String& value) const;
    bool set(const char* key, const char* value) const;
    bool remove(const char* key) const;
    bool has(const char* key) const;
    bool exists() const;
    void del() const;

private:
    void create() const;
    const char* m_file;
};
