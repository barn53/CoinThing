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

    Settings& m_settings;
};
