#include <../C_ESP/General.h>

namespace avp {
  WiFiClient client;
  bool GET_succeed = false;
  String GET_responce;

  void FinishTalk() {
    GET_succeed = true;  GET_responce.clear();
    delay(100);
    while(client.available()) GET_responce += char(client.read());
    if(!client.connected()) client.stop();
  } // FinishTalk

  void StopClient() {
    delay(100);
    client.stop();
    debug_puts("HTML_GET_PRINTF send Failed!");
    while(client.connected()) delay(0);
  } // StopClient

  const String GenerateHTML(const String &html_body, uint16_t AutoRefresh_s, const char *title) {
    String out = "<!DOCTYPE html><html><head>";
    if(title != nullptr) out += "<title>" + String(title) + "</title>";
    if(AutoRefresh_s != 0)
      out += "<meta http-equiv=\"refresh\" content=\"" + String(AutoRefresh_s) + "\">";
    out += "</head><body>" + html_body + "</body></html>";
    return out;
  }  // GenerateAutoRefreshHTML
}

