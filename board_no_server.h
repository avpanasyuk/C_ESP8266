/**
 * @author Sasha
 *
 * @brief class for ESP8266 or ESP32, implements commonly used WiFi functions, including OTA and async server.
 * @details on both board WiFi modules remember the last configuration by it;self, we will rely on it.
 * @note NOTE!!!!!!!!!!!!!! Should add ArduinoOTA.handle() to main loop !!!!!!!!!!!!!!!!!!!!!!!!
 */

#pragma once

#if defined(ESP8266)
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#define WIFI_AUTH_OPEN AUTH_OPEN
#else
#include <WiFi.h>
#endif

#ifndef DO_OTA // !!!!!!!!!!!! DO NOT FORGET TO CALL ArduinoOTA.handle() from the loop()
#define DO_OTA 1 // default on 
#endif

#if DO_OTA
#include <ArduinoOTA.h>
#endif


#include "../C_General/Error.h"

struct ESP_board_no_server {
  enum ConnectionStatus_t {
    IDLE,
    TRYING_TO_CONNECT,
    AP_MODE,
    CONNECTED
  };

  static constexpr uint8_t STR_SIZE = 32;  //< ssid and password string sizes

protected:
  void (*status_indication_func)(enum ConnectionStatus_t);

  IPAddress ip;
  const char *Name;

  void post_connection() {
    ip = WiFi.localIP();
#ifdef DEBUG
    Serial.printf("Connected in STA mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
#endif
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
   * @param default_ssid if stored configuration failed to connect try this one
   * @param default_pass if stored configuration failed to connect try this one
   */
  ESP_board_no_server(const char *Name_,
    void (*status_indication_func_)(enum ConnectionStatus_t),
    const char *default_ssid = nullptr,
    const char *default_pass = nullptr,
    bool ArduinoOTAmDNS = false) : status_indication_func(status_indication_func_), Name(Name_) {
    // if AutoConnect is enabled the WIFI library tries to connect to the last WiFi configuration that it remembers
    // on startup
    if(WiFi.getAutoConnect()) {
      status_indication_func(TRYING_TO_CONNECT);
      WiFi.waitForConnectResult();
    }

    // trying default WiFI configuration if present
    if(!WiFi.isConnected() && default_ssid != nullptr && default_pass != nullptr) {
      WiFi.mode(WIFI_STA);
      WiFi.hostname(Name);
      WiFi.begin(default_ssid, default_pass);
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
#ifdef DEBUG
      Serial.printf("Connecting in AP mode, IP:%s!\n", (String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])).c_str());
#endif
    } else
      post_connection();

#if DO_OTA
    // !NOTE Do not forget to put ArduinoOTA.handle() in the loop()
    ArduinoOTA.onStart([]() {
#ifdef DEBUG
      Serial.println("Start");
#endif
    });
    ArduinoOTA.onEnd([]() {
#ifdef DEBUG
      Serial.println("\nEnd");
#endif
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef DEBUG
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
#endif
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if(error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if(error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if(error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if(error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if(error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
#endif
  }

  String getIP() const { return ip.toString(); }
  void loop() {
#if DO_OTA
    ArduinoOTA.handle();
#endif
  } // loop
};   // ESP_board

