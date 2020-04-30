#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "444-techteam";
const char* password = "00000444";
IPAddress tcp_db;

bool taken[3];

AsyncWebServer server(80);

void setup(){
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>SHARING BOX by 444 Tech Team</title><style>");
    response->print("div { display: flex; }");
    response->print("</style></head><body><h1>SHARING BOX</h1><h3> by 444 Tech Team</h3><h2>Control panel</h2><hr>");
    response->print("<div><p>Sec. 1 (Laptop):</p><p>");
    if(taken[0])
      response->print("Unavailable");
    else
      response->print("Available");
    response->print("</div><button type=\"button\" onclick=\"window.location.href='/unlock1'\">Unlock</button>");
    response->print("<div><p>Sec. 2 (HDMI): </p><p>");
    if(taken[1])
      response->print("Unavailable");
    else
      response->print("Available");
    response->print("</p></div>");
    response->print("<button type=\"button\" onclick=\"window.location.href='/unlock2'\">Unlock</button>");
    response->print("<div><p>Sec. 3 (Remote): </p><p>");
    if(taken[0])
      response->print("Unavailable");
    else
      response->print("Available");
    response->print("</p></div>");
    response->print("<button type=\"button\" onclick=\"window.location.href='/unlock3'\">Unlock</button><hr>");
    response->print("<button type=\"button\" onclick=\"window.location.href='/unlock_all'\">Unlock all</button>");
    response->print("<button type=\"button\" onclick=\"window.location.href='/auth_db'\">Authorize this device as a database</button>");
    response->print("</body></html>");
    request->send(response);
  });
  server.on("/unlock1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("u1");
    request->send_P(200, "text/plain", "Unlocking Sec. 1...");
  });
  server.on("/unlock2", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("_2");
    request->send_P(200, "text/plain", "Unlocking Sec. 2...");
  });
  server.on("/unlock3", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("_3");
    request->send_P(200, "text/plain", "Unlocking Sec. 3...");
  });
  server.on("/unlock_all", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("_a");
    request->send_P(200, "text/plain", "Unlocking all...");
  });
  server.on("/auth_db", HTTP_GET, [](AsyncWebServerRequest *request){
    tcp_db = request->client()->remoteIP();
    request->send_P(200, "text/plain", "Current device is now set as the database server. Sending data to tcp server on port 41.");
  });
  server.begin();
}
 
void loop(){
  WiFiClient tcp;
  tcp.connect(tcp_db, 41);
  String s = Serial.readString();
  if(s[0] == '+') {
    tcp.print(s);
    delay(200);
    tcp.readString();
  }
  else if(s[0] == '?') {
    tcp.print(s);
    delay(200);
    String res = tcp.readString();
    Serial.print(res);
  }
  else if(s[0] == 'u') {
    if(s[1] == '0')
      taken[0] = !taken[0];
    else if(s[1] == '1')
      taken[1] = !taken[1];
    else
      taken[2] = !taken[2];
  }
}
