#pragma once

#if defined(ESP8266)
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESPAsyncTCP.h>
#define WIFI_AUTH_OPEN AUTH_OPEN 
#else
#include <WiFi.h>
#include <AsyncTCP.h>
#endif

#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>

#include <C_General/Error.h>

struct ESP_board {
  enum ConnectionStatus_t { IDLE,
                            TRYING_TO_CONNECT,
                            AP_MODE,
                            CONNECTED };
  static constexpr char Version[] = "0.2";
  static constexpr uint8_t STR_SIZE = 32;  //< ssid and password string sizes
  AsyncWebServer server;

 protected:
  void (*status_indication_func)(enum ConnectionStatus_t);

  String WiFi_Around;
  IPAddress ip;

 public:
  /**
   * @brief initializes esp8266 board
   *
   * @param Name_ c_str name as seen by DNS
   */
  ESP_board(const char *Name,
                void (*status_indication_func_)(enum ConnectionStatus_t),
                const String Usage = "<p><strong>Usage:</strong><br>"
                  "Available URL commands are (like in <em>http://address/command</em>):<ol>"
                  "<li> nothing - outputs this screen</li>"
                  "<li> config?ssid=<em>string</em>&pass=<em>string</em></li></ol></p>",
                const char *default_ssid = nullptr,
                const char *default_pass = nullptr) : server(80), status_indication_func(status_indication_func_),
                                                      WiFi_Around(scan())  {
    // if AutoConnect is enabled the WIFI library tries to connect to the last WiFi configuration that it remembers
    // on startup
    if (WiFi.getAutoConnect()) {
      status_indication_func(TRYING_TO_CONNECT);
      WiFi.waitForConnectResult();
    }

    // trying default WiFI configuration if present
    if (!WiFi.isConnected() && default_ssid != nullptr && default_pass != nullptr) {
      WiFi.mode(WIFI_STA);
      WiFi.hostname(Name);
      WiFi.begin(default_ssid, default_pass);
      status_indication_func(TRYING_TO_CONNECT);
      WiFi.waitForConnectResult();
    }

    // if still not connected switching to AP mode
    if (!WiFi.isConnected()) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(Name, "");
      ip = WiFi.softAPIP();
      status_indication_func(AP_MODE);
      debug_printf("Connecting in AP mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
    } else {
      ip = WiFi.localIP();
      debug_printf("Connected in STA mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
      status_indication_func(CONNECTED);
      WiFi.setAutoConnect(true);
      WiFi.setAutoReconnect(true);
    }

    // setup Web Server
    server.on("/", HTTP_GET, [&,Usage](AsyncWebServerRequest *request) {
      String content;
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = String("<!DOCTYPE HTML>\r\n<html>Hello from <b>") + Name + "</b> at IP: ";
      content += ipStr + ", MAC: " + WiFi.macAddress() + ", Version: " + Version;
      content += Usage;
      content += "<p>WiFi networks:</p>";
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
      if (qsid.length() > 0 && qpass.length() > 0) {
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

    AsyncElegantOTA.begin(&server);  // Start ElegantOTA
    server.begin();
  }

 public:
  static const String scan() {
    String WiFi_Around;
    int n = WiFi.scanNetworks();

    WiFi_Around = "<ol>";
    for (int i = 0; i < n; ++i) {
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

};  // ESP_board

constexpr char ESP_board::Version[];