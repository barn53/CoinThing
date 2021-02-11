#include "handler.h"
#include "gecko.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"
#include <FS.h>

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

#include <ESP8266WebServer.h>
extern ESP8266WebServer server;

using namespace std;

namespace {

String getContentType(const String& filename)
{
    if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".json")) {
        return "application/json";
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

Handler::Handler(const Gecko& gecko, Settings& settings)
    : m_gecko(gecko)
    , m_settings(settings)
{
}

bool Handler::handleReset() const
{
    // todo: handle wifi and settings reset

    server.send(200, "text/plain", "1");
    return true;
}

bool Handler::handleVersion() const
{
    server.send(200, "text/plain", VERSION);
    return true;
}

bool Handler::streamFile(const char* filename)
{
    String contentType = getContentType(filename);
    if (SPIFFS.exists(filename)) {
        File file = SPIFFS.open(filename, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
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

    if (server.hasArg("brightness")) {
        server.send(200, "text/plain", server.arg("brightness").c_str());
        m_settings.setBrightness(static_cast<uint8_t>(server.arg("brightness").toInt()));
    } else {
        Settings::Status status(m_settings.set(m_gecko,
            server.arg("coin").c_str(),
            server.arg("currency").c_str(),
            static_cast<uint8_t>(server.arg("number_format").toInt()),
            static_cast<uint8_t>(server.arg("chart_period").toInt()),
            static_cast<uint8_t>(server.arg("chart_style").toInt()),
            server.arg("heartbeat").toInt() != 0));

        String error;
        switch (status) {
        case Settings::Status::OK:
            if (!streamFile("/settings.json")) {
                error = R"({"error":"file 'settings.json' not found!"})";
            }
            break;
        case Settings::Status::COIN_INVALID:
            error = R"({"error":"Coin ID ')" + server.arg("coin") + R"(' is invalid!"})";
            break;
        case Settings::Status::CURRENCY_INVALID:
            error = R"({"error":"Currency ')" + server.arg("currency") + R"(' is invalid!"})";
            break;
        }

        if (!error.isEmpty()) {
            server.send(200, "application/json", error.c_str());
        }
    }

    return true;
}

bool Handler::handleAction() const
{
    String path(server.uri());
    if (path == "/action/set") {
        return handleSet();
    } else if (path == "/action/reset") {
        return handleReset();
    } else if (path == "/action/version") {
        return handleVersion();
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
    return streamFile(path.c_str());
}
