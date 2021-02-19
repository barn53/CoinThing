#pragma once
#include <Arduino.h>

class Gecko;
class Settings;

class Handler {
public:
    Handler(const Gecko& gecko, Settings& settings);

    bool handleAction() const;
    static bool handleFileRead();

private:
    static bool streamFile(const char* filename);

    bool handleSet() const;
    bool handleResetESP() const;
    bool handleResetSettings() const;
    bool handleResetWiFi() const;
    bool handleResetAll() const;
    bool handleGetVersion() const;
    bool handleGetName() const;
    bool handleForUpdate() const;

    const Gecko& m_gecko;
    Settings& m_settings;
};
