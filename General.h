#pragma once

#include <WiFiClientSecure.h>
#include <C_General/Error.h>

namespace avp {
  static constexpr unsigned long DefaultTimeout_ms = 20000; 
  /**
   *@brief sending GET request
   *
   * @param Message - like "/pin?i=5&set=1"
   * @retval pointer to a static string which contains return message with headers stripped. It will be modified on the next call
   *    or nullptr if request was unsuccessful
   */
  const char *SendGET(const char *server, const char *Message, uint16_t port = 80, unsigned long Timeout = DefaultTimeout_ms);
  const char *SendGET_Secure(const char *server, const char *Message, uint16_t port = 443, unsigned long Timeout = DefaultTimeout_ms);
  // ***************** INTERNET CLIENT CONNECTION ************************************
  const String &GenerateHTML(const char *html_body, uint16_t AutoRefresh_s = 0, const char *title = nullptr);
} // namespace avp


