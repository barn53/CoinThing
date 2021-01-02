#pragma once
#include <Arduino.h>

class Settings;

class Handler {
public:
    Handler(Settings& settings);

    bool handleAction() const;

    static bool handleFileRead();

private:
    bool handleSet() const;
    bool handleGet(const String& action) const;
    bool handleCheck(const String& check) const;

    Settings& m_settings;
};
