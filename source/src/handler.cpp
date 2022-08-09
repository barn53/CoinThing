#include "handler.h"
#include "common.h"
#include "gecko.h"
#include "handler.h"
#include "json_store.h"
#include "pre.h"
#include "settings.h"
#include "utils.h"

#include <ESP8266WebServer.h>
extern ESP8266WebServer server;
extern String HostName;
extern JsonStore Secrets;

using namespace std;

namespace {

String getContentType(const String& filename)
{
    if (filename.endsWith(".html")) {
        return F("text/html");
        //    } else if (filename.endsWith(".gz")) {
        //        return F("application/x-gzip");
    } else if (filename.endsWith(".css")) {
        return F("text/css");
    } else if (filename.endsWith(".json")) {
        return F("application/json");
    } else if (filename.endsWith(".js")) {
        return F("application/javascript");
    } else if (filename.endsWith(".ico")) {
        return F("image/x-icon");
    } else if (filename.endsWith(".jpg")) {
        return F("image/jpeg");
    } else if (filename.endsWith(".bmp")) {
        return F("image/bmp");
    }
    return F("text/plain");
}

} // anonymous namespace

Handler::Handler(Gecko& gecko, Settings& settings)
    : m_gecko(gecko)
    , m_settings(settings)
{
}

bool Handler::handleResetESP() const
{
    server.send(200, F("text/plain"), "1");

    delay(200);
    ESP.restart();
    return true;
}

bool Handler::handleResetSettings() const
{
    server.send(200, F("text/plain"), "1");

    m_settings.deleteFile();
    delay(200);
    ESP.restart();
    return true;
}

bool Handler::handleResetWiFi() const
{
    server.send(200, F("text/plain"), "1");

    Secrets.remove(F("ssid"));
    Secrets.remove(F("pwd"));
    delay(200);
    WiFi.disconnect();
    delay(200);
    ESP.restart();
    return true;
}

bool Handler::handleResetAll() const
{
    server.send(200, F("text/plain"), "1");

    m_settings.deleteFile();
    Secrets.remove(F("ssid"));
    Secrets.remove(F("pwd"));
    SPIFFS.remove(FAKE_GECKO_SERVER_FILE);
    // keep COLOR_SET_FILE
    delay(200);
    WiFi.disconnect();
    delay(200);
    ESP.restart();
    return true;
}

bool Handler::handleGetVersion() const
{
    server.send(200, F("text/plain"), VERSION);
    return true;
}

bool Handler::handleGetName() const
{
    server.send(200, F("text/plain"), HostName);
    return true;
}

bool Handler::handleGetPrice()
{
    gecko_t price;
    gecko_t price2;
    gecko_t change_pct;
    m_gecko.price(0, price, price2, change_pct);

    String result;
    formatNumber(price, result, m_settings.numberFormat(), false, true, m_settings.smallDecimalNumberFormat());
    String ret;
    ret = m_settings.symbol(0);
    ret += F(": ");
    ret += result;
    ret += m_settings.currencySymbol();
    server.send(200, F("text/plain"), ret);
    return true;
}

bool Handler::handleEncrypt()
{
    if (server.hasArg(F("value"))) {
        String encrypted(encryptEncode(server.arg(F("value"))));
        String ret;
        ret = encrypted;
        ret += "\n";
        ret += decodeDecrypt(encrypted);
        server.send(200, F("text/plain"), ret);
    }
    return true;
}

bool Handler::handleDecrypt()
{
    if (server.hasArg(F("value"))) {
        String decrypted(decodeDecrypt(server.arg(F("value"))));
        String ret;
        ret = decrypted;
        ret += "\n";
        ret += encryptEncode(decrypted);
        server.send(200, F("text/plain"), ret);
    }
    return true;
}

bool Handler::handleForUpdate() const
{
    server.send(200, F("text/plain"), "1");

    // if file exists on powerup, ESP goes into update mode
    File f = SPIFFS.open(FOR_UPDATE_FILE, "w");
    f.close();

    delay(200);
    ESP.restart();
    return true;
}

bool Handler::handleSelftest() const
{
    server.send(200, F("text/plain"), "1");

    // if file exists on powerup, ESP starts selftest
    File f = SPIFFS.open(SELFTEST_FILE, "w");
    f.close();

    delay(200);
    ESP.restart();
    return true;
}

bool Handler::streamFile(const char* filename)
{
    LOG_FUNC
    LOG_I_PRINT("  File: ")
    LOG_PRINT(filename)

    String contentType = getContentType(filename);
    String filename_found;
    String filename_gz(filename);
    filename_gz += ".gz";

    if (SPIFFS.exists(filename_gz)) {
        LOG_PRINT(" found .gz: ")
        LOG_PRINT(filename_gz)
        filename_found = filename_gz;
    } else if (SPIFFS.exists(filename)) {
        filename_found = filename;
    } else {
        return false;
    }

    File file = SPIFFS.open(filename_found, "r");
    server.sendHeader(F("Access-Control-Allow-Origin"), "*");
    server.streamFile(file, contentType);
    file.close();
    LOG_PRINTLN(" - ok");
    return true;

    LOG_PRINTLN(" - does not exist!")
    return false;
}

bool Handler::handleSet() const
{
#if COIN_THING_SERIAL > 0
    LOG_I_PRINTLN("handleSet: parsed Query:");
    for (int ii = 0; ii < server.args(); ++ii) {
        LOG_I_PRINT(server.argName(ii));
        LOG_PRINT(" -> ");
        LOG_PRINTLN(server.arg(ii));
    }
#endif

    if (server.hasArg(F("brightness"))) {
        m_settings.setBrightness(static_cast<uint8_t>(server.arg(F("brightness")).toInt()));
        streamFile(BRIGHTNESS_FILE);
    } else if (server.hasArg(F("json"))) {
        m_settings.set(server.arg(F("json")).c_str());
        streamFile(SETTINGS_FILE);
    } else if (server.hasArg(F("colorset"))) {
        m_settings.setColorSet(server.arg(F("colorset")).toInt());
        server.send(200, F("text/plain"), server.arg(F("colorset")));
        delay(200);
        ESP.restart();
    } else if (server.hasArg(F("fakegeckoserver"))) {
        m_settings.setFakeGeckoServer(server.arg(F("fakegeckoserver")));
        server.send(200, F("text/plain"), server.arg(F("fakegeckoserver")));
        delay(200);
        ESP.restart();
    } else if (server.hasArg(F("proapi"))) {
        m_gecko.enableProAPI(server.arg(F("proapi")) == F("1") ? true : false);
        server.send(200, F("text/plain"), server.arg(F("proapi")) == F("1") ? F("1") : F("0"));
        delay(200);
    } else {
        server.send(200, F("application/json"), F(R"({"error":"Nothing to set!"})"));
    }

    return true;
}

bool Handler::handleAction()
{
    String path(server.uri());
    LOG_I_PRINTF("handleAction: path: %s\n", path.c_str());

    if (path == F("/action/set")) {
        return handleSet();
    } else if (path == F("/action/reset/esp")) {
        return handleResetESP();
    } else if (path == F("/action/reset/settings")) {
        return handleResetSettings();
    } else if (path == F("/action/reset/wifi")) {
        return handleResetWiFi();
    } else if (path == F("/action/reset/all")) {
        return handleResetAll();
    } else if (path == F("/action/reset/forupdate")) {
        return handleForUpdate();
    } else if (path == F("/action/selftest")) {
        return handleSelftest();
    } else if (path == F("/action/get/version")) {
        return handleGetVersion();
    } else if (path == F("/action/get/name")) {
        return handleGetName();
    } else if (path == F("/action/get/price")) {
        return handleGetPrice();
    } else if (path == F("/action/encrypt")) {
        return handleEncrypt();
    } else if (path == F("/action/decrypt")) {
        return handleDecrypt();
    }
    return false;
}

bool Handler::handleFileRead()
{
    LOG_FUNC
    String path(server.uri());
    LOG_I_PRINTF("handleFileRead: %s\n", path.c_str());

    if (path.endsWith(F("/"))) {
        path += F("settings.html");
    }

    if (path.endsWith(F("/secrets.json"))) {
        server.send(403, F("text/plain"), F("403: Forbidden"));
        return true;
    }
    return streamFile(path.c_str());
}
