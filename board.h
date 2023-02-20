/**
 * @author Sasha
 *
 * @brief class for ESP8266 or ESP32, implements commonly used WiFi functions, including OTA and async server.
 * @details on both board WiFi modules remember the last configuration by it;self, we will rely on it.
 */

#pragma once

#if defined(ESP8266)
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESPAsyncTCP.h>
#define WIFI_AUTH_OPEN AUTH_OPEN
#else
#include <AsyncTCP.h>
#include <WiFi.h>
#endif

#include <AsyncElegantOTA.h>
#include <C_General/Error.h>
#include <ESPAsyncWebServer.h>

struct ESP_board {
  enum ConnectionStatus_t {
    IDLE,
    TRYING_TO_CONNECT,
    AP_MODE,
    CONNECTED
  };
  static constexpr char Version[] = "0.4";
  static constexpr uint8_t STR_SIZE = 32;  //< ssid and password string sizes
  AsyncWebServer server;

protected:
  void (*status_indication_func)(enum ConnectionStatus_t);

  String WiFi_Around;
  IPAddress ip;
  const char *Name;

  void post_connection() {
    ip = WiFi.localIP();
    debug_printf("Connected in STA mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
    status_indication_func(CONNECTED);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }  // post_connection

public:
  /**
   * @brief initializes esp8266 or esp32 board
   *
   * @param Name_ c_str name as seen by DNS
   * @param status_indication_func_ function which will be called by the class when commection status changes
   * @param AddUsage html code for additional commands in "Usage:" descrition, each line starts with "<li>" and ends with "</li>"
   * @param default_ssid if stored configuration failed to connect try this one
   * @param default_pass if stored configuration failed to connect try this one
   */
  ESP_board(const char *Name_,
    void (*status_indication_func_)(enum ConnectionStatus_t),
    const String AddUsage = "",
    const char *default_ssid = nullptr,
    const char *default_pass = nullptr): server(80), status_indication_func(status_indication_func_), WiFi_Around(scan()),
    Name(Name_) {
    // if AutoConnect is enabled the WIFI library tries to connect to the last WiFi configuration that it remembers
    // on startup
    if(WiFi.getAutoConnect()) {
      status_indication_func(TRYING_TO_CONNECT);
      WiFi.waitForConnectResult();
    }

    // trying default WiFI configuration if present
    if(!WiFi.isConnected() && default_ssid != nullptr && default_pass != nullptr) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(default_ssid, default_pass);
      WiFi.setHostname(Name);
      status_indication_func(TRYING_TO_CONNECT);
      WiFi.waitForConnectResult();
    } else
      post_connection();

    // if still not connected switching to AP mode
    if(!WiFi.isConnected()) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(Name, "");
      ip = WiFi.softAPIP();
      status_indication_func(AP_MODE);
      debug_printf("Connecting in AP mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
    } else
      post_connection();

    // setup Web Server
    server.on("/", HTTP_GET, [&, AddUsage](AsyncWebServerRequest *request) {
      String content;
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      debug_printf(Name);
      content = String("<!DOCTYPE HTML>\r\n<html>Hello from <b>") + Name + "</b> at IP: ";
      content += ipStr + ", MAC: " + WiFi.macAddress() + ", Version: " + Version;
      content += "<p><strong>Usage:</strong><br>"
        "Available URL commands are (like in <em>http://address/command</em>):<ol>"
        "<li> nothing - outputs this screen</li>"
        "<li> pin?i=n - return pin n settings</li>"
        "<li> pin?i=n[&set=(0|1)] - set pin value</li>"
        "<li> pin?i=n[&mode=(0|1)] - set pin mode</li>"
        "<li> config?ssid=<em>string</em>&pass=<em>string</em></li>"
        "<li> reset - reboots MCU</li>"
        "<li> update - update firmware page</li>";
      content += AddUsage;
      content += "</ol></p><p>WiFi networks:</p>";
      content += "<p>";
      content += WiFi_Around;
      content += String("</p><form method='get' action='config'><label>SSID: </label><input name='ssid' length=") + (STR_SIZE - 1) +
        " value='" + WiFi.SSID() + "'><input name='pass' length=" + (STR_SIZE - 1) +
        "><input type='submit'></html>";
      request->send(200, "text/html", content);
    });

    server.on("/config", HTTP_GET, [&](AsyncWebServerRequest *request) {  // URL xxx.xxx.xxx.xxx/set?pin=14&value=1
      String qsid = request->arg("ssid");
      String qpass = request->arg("pass");
      if(qsid.length() > 0 && qpass.length() > 0) {
        request->send(200, "text/plain", "WiFI configuration changed, connection is being reistablished!");
        delay(1000);
        WiFi.disconnect();
        delay(1000);
        WiFi.mode(WIFI_STA);
        WiFi.begin(qsid.c_str(), qpass.c_str());
        status_indication_func(TRYING_TO_CONNECT);
        WiFi.setAutoConnect(WiFi.waitForConnectResult() == WL_CONNECTED);
        delay(1000);
        ESP.restart();
      }
    });

    server.on("/pin", HTTP_GET, [&](AsyncWebServerRequest *request) {  // URL xxx.xxx.xxx.xxx/pin?i=n[&analog][&set=x][&mode=x]
      if(request->hasArg("i")) {
        uint8_t Pin = request->arg("i").toInt();
        bool Analog = request->hasArg("analog");
        if(request->hasArg("set") || request->hasArg("mode")) {
          if(request->hasArg("mode")) {
            pinMode(Pin, request->arg("mode").toInt());
            request->send(200, "text/plain", "Pin mode is set!");
          }
          if(request->hasArg("set")) {
            if(Analog)
              analogWrite(Pin, request->arg("set").toInt());
            else
              digitalWrite(Pin, request->arg("set").toInt());
            request->send(200, "text/plain", "Pin is set!");
          }
        } else {
          if(Analog)
            request->send(200, "text/plain", String("Analog pin #") + Pin + " reads " + analogRead(Pin));
          else
            request->send(200, "text/plain", String("Digital pin #") + Pin + " reads " + digitalRead(Pin));
        }
      } else
        request->send(200, "text/plain", "No pin index!");
    });

    server.on("/reset", HTTP_GET, [&](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Resetting ...");
      delay(1000);
      ESP.restart();
    });

    AsyncElegantOTA.begin(&server);  // Start ElegantOTA
    server.begin();
  }

public:
  static const String scan() {
    String WiFi_Around;
    int n = WiFi.scanNetworks();

    WiFi_Around = "<ol>";
    for(int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      WiFi_Around += "<li>";
      WiFi_Around += WiFi.SSID(i);
      WiFi_Around += " (";
      WiFi_Around += WiFi.RSSI(i);

      WiFi_Around += ")";
      WiFi_Around += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
      WiFi_Around += "</li>";
    }
    WiFi_Around += "</ol>";
    return WiFi_Around;
  }  // scan
};   // ESP_board

constexpr char ESP_board::Version[];

static const String GenerateHTML(const String &html_body, uint16_t AutoRefresh_s = 0, const char *title = nullptr) {
  String out = "<!DOCTYPE html><html><head>";
  if(title != nullptr) out += "<title>" + String(title) + "</title>";
  if(AutoRefresh_s != 0)
    out += "<meta http-equiv=\"refresh\" content=\"" + String(AutoRefresh_s) + "\">";
  out += "</head><body>" + html_body + "</body></html>";
  return out;
}  // GenerateAutoRefreshHTML

// following define disables interrupts but enables them does not matter how function has returned
#define PAUSE_INTERRUPTS struct _t { _t() { noInterrupts(); } ~_t() { interrupts(); } } _;