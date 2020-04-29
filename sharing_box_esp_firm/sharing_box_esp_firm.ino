#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "444-techteam";
const char* password = "00000444";

// current temperature & humidity, updated in loop()
bool taken[3];

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
    // TODO: rewrite index-html interaction block
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
  server.begin();
}
 
void loop(){
  char *s = Serial.readString();
  if(s[0] == "+") {
    //add rec to database;
  }
  else if(s[0] == "?") {
    //request rec from database;
    if(/*response positive*/) {
      Serial.print("1");
    }
    else {
      Serial.print("0");
    }
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
