#include <../C_ESP/General.h>

WiFiClient avp::client;
bool avp::GET_succeed = false;
String avp::GET_responce;

void avp::FinishTalk() {
  String GET_responce;
  GET_succeed = true;  GET_responce.clear();
  delay(100);
  while(client.available()) GET_responce += client.read();
  if(!client.connected()) client.stop();
} // FinishTalk

void avp::StopClient() {
  delay(100);
  client.stop();
  debug_puts("Send Failed!");
  while(client.connected()) delay(0);
} // StopClient

const String avp::GenerateHTML(const String &html_body, uint16_t AutoRefresh_s, const char *title) {
  String out = "<!DOCTYPE html><html><head>";
  if(title != nullptr) out += "<title>" + String(title) + "</title>";
  if(AutoRefresh_s != 0)
    out += "<meta http-equiv=\"refresh\" content=\"" + String(AutoRefresh_s) + "\">";
  out += "</head><body>" + html_body + "</body></html>";
  return out;
}  // GenerateAutoRefreshHTML
