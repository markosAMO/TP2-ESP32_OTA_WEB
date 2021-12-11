#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include "QueryLib.h"
const char* ssid = "ESP32_AP";
const char* password = "123456789";
const char* www_username = "admin";
const char* www_password = "123456789";
const char* version = "1.0.0";
String variable;
/*
 * Declaramos objeto de la libreria WebServer
 */
 
 WebServer server(80);

 /*
 * Para generar codigo jQuery
 */

void onJavaScript(void) {
    Serial.println("onJavaScript(void)");
    server.setContentLength(jquery_min_js_v3_2_1_gz_len);
    server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
}

void SetupServer() {
  
  server.on("/version", HTTP_GET, [](){
    server.sendHeader("Connection", "close");
    server.send(200,"text/plain",version);
  });

  /*
   * Manejo del endpoint '/update' para subir el archivo
   */

  server.on("/update", HTTP_POST, []() {
      //server.sendHeader("Connection", "close");
      //server.send(200, "text/html", (Update.hasError()) ? "FAIL" : "OK");
      Serial.println("conecction successful");
      Serial.println(server.argName(0));
      String var = server.arg(("plain"));
      Serial.println();
      server.send(200, "text/plain", String("POST realizado"));

  });


}

void setup(void) {

  Serial.begin(115200);

  /*
   * Se configura el ESP32 como Access Point
   */

  delay(10);
  Serial.print("Seteando WiFi en modo Access Point");
  WiFi.mode(WIFI_AP);
  while(!WiFi.softAP(ssid, password))
  {
   Serial.println(".");
    delay(100);
  }

  Serial.print("Iniciado AP ");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  SetupServer();

  server.begin();
  

}

void loop() {

  server.handleClient();
  ArduinoOTA.handle();

}