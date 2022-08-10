#pragma once
#include <Arduino.h>

class Handler {
public:
    bool handleAction();
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
    bool handleGetPrice();
    bool handleEncrypt();
    bool handleDecrypt();

    bool handleForUpdate() const;
    bool handleSelftest() const;
};
