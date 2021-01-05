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

bool Handler::handleCheck(const String& check) const
{
    Serial.printf("handleAction: check %s\n", check.c_str());

    if (check.startsWith("coin/")) {
        String coin(check.substring(5).c_str());
        if (m_settings.getGecko().isValidCoin(coin)) {
            server.send(200, "text/plain", "1");
        } else {
            server.send(200, "text/plain", "0");
        }
        return true;
    } else if (check.startsWith("currency/")) {
        String currency(check.substring(9).c_str());
        if (m_settings.getGecko().isValidCurrency(currency)) {
            server.send(200, "text/plain", "1");
        } else {
            server.send(200, "text/plain", "0");
        }
        return true;
    }

    return false;
}

bool Handler::handleSet() const
{
#if 0
    Serial.printf("handleAction: set %s\n", query.c_str());

    const String& pathArg(unsigned int i) const; // get request path argument by number
    const String& arg(const String& name) const; // get request argument value by name
    const String& arg(int i) const; // get request argument value by number
    const String& argName(int i) const; // get request argument name by number
    int args() const; // get arguments count
    bool hasArg(const String& name) const; // check if argument exists

    String mutabl(query);
    std::vector<std::pair<const char*, const char*>> vec;
    parseQuery(mutabl.begin(), vec);

#endif

    Serial.println("Parsed Query:");

    for (int ii = 0; ii < server.args(); ++ii) {
        Serial.print(server.argName(ii));
        Serial.print(" -> ");
        Serial.println(server.arg(ii));
    }

    Settings::Status status(m_settings.set(server.arg("coin").c_str(),
        server.arg("currency").c_str(),
        static_cast<uint8_t>(server.arg("number_format").toInt())));

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

bool Handler::handleGet(const String& action) const
{
    Serial.printf("handleAction: get %s\n", action.c_str());

    if (action == "coin") {
        server.send(200, "text/plain", m_settings.coin());
        return true;
    } else if (action == "currency") {
        server.send(200, "text/plain", m_settings.currency());
        return true;
    } else if (action == "number_format") {
        String s(static_cast<uint8_t>(m_settings.numberFormat()));
        server.send(200, "text/plain", s);
        return true;
    }

    return false;
}

bool Handler::handleAction() const
{
    String path(server.uri());
    if (path.startsWith("/action/get/")) {
        String action(path.substring(12));
        return handleGet(action);
    } else if (path.startsWith("/action/check/")) {
        String action(path.substring(14));
        return handleCheck(action);
    } else if (path == "/action/set") {
        return handleSet();
    }
    return false;
}

bool Handler::handleFileRead()
{
    String path(server.uri());
    Serial.printf("handleFileRead: %s\n", path.c_str());
    if (path.endsWith("/")) {
        path += "settings.html"; // If a folder is requested, send the settings file
        // Serial.printf(" --> %s\n", p.c_str());
    }
    String contentType = getContentType(path); // Get the MIME type

    if (SPIFFS.exists(path)) { // If the file exists
        File file = SPIFFS.open(path, "r"); // Open it
        // size_t sent =
        server.streamFile(file, contentType); // And send it to the client
        file.close(); // Then close the file again
        // Serial.printf("\tSent %u bytes\n", sent);
        return true;
    }
    // Serial.print("\nFile Not Found\n");
    return false; // If the file doesn't exist, return false
}
