#pragma once

#include <WiFiClient.h>
#include <../C_General/Error.h>

namespace avp {
  // ***************** INTERNET CLIENT CONNECTION ************************************
  extern WiFiClient client;
  extern bool GET_succeed;
  extern String GET_responce;

  void FinishTalk();

  void StopClient();

  const String GenerateHTML(const String &html_body, uint16_t AutoRefresh_s = 0, const char *title = nullptr);
} // namespace avp

#define HTML_GET_PRINTF(server, format,...) do{ avp::GET_succeed = false; \
  if(avp::client.connect(server, 80)) {\
    if(avp::client.printf("GET " format " HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", \
      ##__VA_ARGS__, server) > 0) \
      avp::FinishTalk(); else  avp::StopClient(); \
  } else debug_puts("HTML_GET_PRINTF connect failed!"); }while(0)


