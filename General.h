#pragma once

#if defined(AVP_SECURE_GET)
#include <WiFiClientSecure.h>
#else
#include <WiFiClient.h>
#endif
#include <../C_General/Error.h>

namespace avp {
  // ***************** INTERNET CLIENT CONNECTION ************************************
#if defined(AVP_SECURE_GET)
  extern WiFiClientSecure client; // DO NOT FORGET TO PUT avp::client.setInsecure(); at 
  // the end of setup()
  static constexpr uint16_t ServerPort = 443;
#else
  extern WiFiClient client;
  static constexpr uint16_t ServerPort = 80;
#endif

  extern bool GET_succeed;
  extern String GET_responce;

  void FinishTalk();

  void StopClient();

  const String GenerateHTML(const String &html_body, uint16_t AutoRefresh_s = 0, const char *title = nullptr);
} // namespace avp

#define HTML_GET_PRINTF(server, format,...) do{ avp::GET_succeed = false; \
  if(avp::client.connect(server, avp::ServerPort)) {\
    if(avp::client.printf("GET "  format " HTTP/1.1\r\nHost: %s\r\nUser-Agent: (panasyuk@yahoo.com)\r\nConnection: close\r\n\r\n", \
      ##__VA_ARGS__, server) > 0) avp::FinishTalk(); else  avp::StopClient(); \
  } else Serial.println("HTML_GET_PRINTF connect failed!"); }while(0)


