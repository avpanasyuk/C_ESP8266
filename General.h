#pragma once

#include <WiFiClientSecure.h>
#include <../C_General/Error.h>

namespace avp {
  /**
   *@brief sending GET request
   *
   * @param Message - like "/pin?i=5&set=1"
   */
  const char *SendGET(const char *server, const String &Message, uint16_t port = 80, unsigned long Timeout = 20000);
  const char *SendGET_Secure(const char *server, const String &Message, uint16_t port = 443, unsigned long Timeout = 20000);
  // ***************** INTERNET CLIENT CONNECTION ************************************
  const String &GenerateHTML(const String &html_body, uint16_t AutoRefresh_s = 0, const char *title = nullptr);
} // namespace avp


