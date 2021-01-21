#include "handler.h"
#include "gecko.h"
#include "settings.h"
#include "utils.h"
#include <FS.h>

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
extern ESP8266WebServer server;
#else
#include <HTTPClient.h>
#include <SPIFFS.h> // ESP32 only
#include <WebServer.h>
#include <WiFi.h>
extern WebServer server;
#endif

using namespace std;

namespace {

String getContentType(const String& filename)
{
    if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    }
    return "text/plain";
}

} // anonymous namespace

Handler::Handler(Settings& settings)
    : m_settings(settings)
{
}

bool Handler::handleSet() const
{
#if COIN_THING_SERIAL == 1
    Serial.printf("handleAction: set - parsed Query:\n");
    for (int ii = 0; ii < server.args(); ++ii) {
        Serial.print(server.argName(ii));
        Serial.print(" -> ");
        Serial.println(server.arg(ii));
    }
#endif

    Settings::Status status(m_settings.set(server.arg("coin").c_str(),
        server.arg("currency").c_str(),
        static_cast<uint8_t>(server.arg("number_format").toInt()),
        static_cast<uint8_t>(server.arg("chart").toInt()),
        server.arg("heartbeat").toInt() != 0));

    String error;
    switch (status) {
    case Settings::Status::OK:
        server.send(200, "text/plain", "1");
        break;
    case Settings::Status::COIN_INVALID:
        error = "Coin ID '" + server.arg("coin") + "' is invalid!";
        server.send(200, "text/plain", error.c_str());
        break;
    case Settings::Status::CURRENCY_INVALID:
        error = "Currency '" + server.arg("currency") + "' is invalid!";
        server.send(200, "text/plain", error.c_str());
        break;
    }

    return true;
}

bool Handler::handleAction() const
{
    String path(server.uri());
    if (path == "/action/set") {
        return handleSet();
    }
    return false;
}

bool Handler::handleFileRead()
{
    String path(server.uri());
    Serial.printf("handleFileRead: %s\n", path.c_str());
    if (path.endsWith("/")) {
        path += "settings.html";
    }
    String contentType = getContentType(path);

    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}
