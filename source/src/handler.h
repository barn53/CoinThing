#pragma once
#include <Arduino.h>

class Gecko;
class Settings;
class SettingsV12;

class Handler {
public:
    Handler(Gecko& gecko, Settings& settings, SettingsV12& settingsv12);

    bool handleAction();
    static bool handleFileRead();

private:
    static bool streamFile(const char* filename);

    bool handleSet() const;
    bool handleSetV12() const;
    bool handleResetESP() const;
    bool handleResetSettings() const;
    bool handleResetWiFi() const;
    bool handleResetAll() const;

    bool handleGetVersion() const;
    bool handleGetName() const;
    bool handleGetPrice();

    bool handleForUpdate() const;

    Gecko& m_gecko;
    Settings& m_settings;

    SettingsV12& m_settingsv12;
};
