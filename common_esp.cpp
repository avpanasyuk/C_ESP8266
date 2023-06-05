#define _GNU_SOURCE
#include <cstring>
#include "C_General\General.h"
#include "C_ESP\General.h"

namespace avp {
  const String &GenerateHTML(const String &html_body, uint16_t AutoRefresh_s, const char *title) {
    static String out;
    out.clear();
    out += F("<!DOCTYPE html><html><head>");
    if(title != nullptr) out += "<title>" + String(title) + "</title>";
    if(AutoRefresh_s != 0)
      out += "<meta http-equiv=\"refresh\" content=\"" + String(AutoRefresh_s) + "\">";
    out += "</head><body>" + html_body + "</body></html>";
    return out;
  }  // GenerateAutoRefreshHTML

/**
 *@brief sending GET request
 *
 * @param Message - like "/pin?i=5&set=1"
 */
  static const char *SendGET_(WiFiClient *pClient, const char *server, const String &Message, uint16_t port,
    uint32_t Timeout_ms = 10000) {
    static String GET_responce;

    pClient->setTimeout(Timeout_ms);

    if(pClient->connect(server, port)) {
      pClient->printf("GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: ff\r\nConnection: close\r\n\r\n", Message.c_str(), server);
      
      // we are waiting until either response reaches or server closes connection
      auto Timeout = millis() + Timeout_ms;
      GET_responce.clear();
      while(pClient->connected() && millis() < Timeout)
        while(pClient->available())
          GET_responce += char(pClient->read());

      pClient->stop();
      static const char *cl = "content-length:";
      const char *r = GET_responce.c_str();
      const char *p = strcasestr(r, cl);
      if(p != nullptr) {
        p += strlen(cl);
        
        unsigned int Length;
        if(sscanf(p, "%u", &Length) == 1) {
          // debug_printf("%s<br>%u<br>%s", r, Length, r + (strlen(r) - Length));
          return r + (strlen(r) - Length);
        }  else debug_printf("Failed to parse '%s'!", p);
      } else debug_printf("Cannot find '%s' in '%s' (case ignored)", cl, r);
    } else debug_printf("Failed to connect to '%s'!", server);
    return nullptr;
  } // CheckPump

/**
 *@brief sending GET request
 *
 * @param Message - like "/pin?i=5&set=1"
 */
  const char *SendGET(const char *server, const String &Message, uint16_t port, unsigned long Timeout) {
    static WiFiClient c;
    c.setTimeout(Timeout);
    return SendGET_(&c, server, Message, port, Timeout);
  } // SendGET

  const char *SendGET_Secure(const char *server, const String &Message, uint16_t port, unsigned long Timeout) {
    static WiFiClientSecure c;
    c.setInsecure();
    c.setTimeout(Timeout);
    return SendGET_(&c, server, Message, port, Timeout);
  } // SendGET_Secure
}

