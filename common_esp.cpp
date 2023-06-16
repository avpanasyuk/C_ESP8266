#include <cstring>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "C_General\General_C.h"
// #include "C_ARDUINO\General.h"
#include "C_ESP\General.h"

namespace avp {
  const String &GenerateHTML(const char *html_body, uint16_t AutoRefresh_s, const char *title) {
    static String out;
    if(!out) out.reserve(6000);
    out.clear();
    out += F("<!DOCTYPE html><html><head>");
    if(title != nullptr) {
      out += "<title>";
      out += title;
      out += "</title>";
    }
    if(AutoRefresh_s != 0) {
      out += F("<meta http-equiv=\"refresh\" content=\"");
      out += AutoRefresh_s;
      out += "\">";
    }
    out += "</head><body>";
    // out += avp::urlencode(html_body);
    out += html_body;
    out += "</body></html>";
  #ifdef DEBUG
    // Serial.println(out);
  #endif
    return out;
  }  // GenerateAutoRefreshHTML

/**
 *@brief sending GET request
 *
 * @param Message - like "/pin?i=5&set=1"
 * @retval char *response_message if no error 
 */
  static const char *SendGET_(WiFiClient *pClient, const char *server, const char *Message, uint16_t port,
    uint32_t Timeout_ms) {
    static String GET_response;
    pClient->setTimeout(Timeout_ms);

    if(pClient->connect(server, port)) {
      pClient->printf("GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: ff\r\nConnection: close\r\n\r\n", Message, server);
      
      // we are waiting until either response reaches or server closes connection
      auto Timeout = millis() + Timeout_ms;
      GET_response.clear();
      while(pClient->connected() && millis() < Timeout)
        while(pClient->available())
          GET_response += char(pClient->read());
      

#ifdef DEBUG
      debug_puts(GET_response.c_str());
#endif      

      pClient->stop();
      static const char *headers_end = "\r\n\r\n";
      const char *response_message = strstr(GET_responce.c_str(), headers_end);
      if(response_message != nullptr) return response_message + strlen(headers_end);
    } else debug_printf("Failed to connect to '%s'!", server);
    return nullptr;
  } // CheckPump

/**
 *@brief sending GET request
 *
 * @param Message - like "/pin?i=5&set=1"
 */
  const char *SendGET(const char *server, const char *Message, uint16_t port, unsigned long Timeout) {
    static WiFiClient c;
    return SendGET_(&c, server, Message, port, Timeout);
  } // SendGET

  const char *SendGET_Secure(const char *server, const char *Message, uint16_t port, unsigned long Timeout) {
    static WiFiClientSecure c;
    c.setInsecure();
    return SendGET_(&c, server, Message, port, Timeout);
  } // SendGET_Secure
}

