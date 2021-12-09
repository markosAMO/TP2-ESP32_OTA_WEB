#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include "QueryLib.h"
#include <HTTPClient.h>
const char* version = "1.1.1v";
const char* ssid = "ESP32_AP";
const char* password = "123456789";
const char* www_username = "admin";
const char* www_password = "123456789";
#define HOST "http://127.0.0.1:5000/show-version"
HTTPClient client;
int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware

/*
 * Declaramos objeto de la libreria WebServer
 */

 WebServer server(80);

void onJavaScript(void) {
    Serial.println("onJavaScript(void)");
    server.setContentLength(jquery_min_js_v3_2_1_gz_len);
    server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
}
void WriteFirmware(uint8_t *data, size_t len){
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}
void UpdateFirmware (){
    client.begin(HOST);
    // Get file, just to check if each reachable
    int resp = 0;//client.GET();
    Serial.print("Response: ");
    Serial.println(resp);
    // If file is reachable, start downloading
    if(resp > 0){
        // get length of document (is -1 when Server sends no Content-Length header)
        totalLength = client.getSize();
        // transfer to local variable
        int len = totalLength;
        // this is required to start firmware update process
        Update.begin(UPDATE_SIZE_UNKNOWN);
        Serial.printf("FW Size: %u\n",totalLength);
        // create buffer for read
        uint8_t buff[128] = { 0 };
        // get tcp stream
        WiFiClient * stream = client.getStreamPtr();
        // read all data from server
        Serial.println("Updating firmware...");
        while(client.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();
            if(size) {
                // read up to 128 byte
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                // pass to function
                WriteFirmware(buff, c);
                if(len > 0) {
                  len -= c;
                }
            }
            delay(1);
        }
    }else{
      Serial.println("Cannot download firmware file");
    }
    client.end();
}

void SendVersion (){
    client.begin(HOST);
    client.addHeader("Content-Type", "text/plain");
    int response = client.POST(version);
    if(response>0){
  
      String response = client.getString();  //Get the response to the request
    
      Serial.println(response);   //Print return code
      Serial.println(response);           //Print request answer

    }else{
  
      Serial.print("Error on sending POST: ");
      Serial.println(response);
  
    }
    client.end(); 
}

void SetupOta() {

  /*
   * Se incluye seguridad MD5 y manejo de errores al hacer un update OTA
   */

  ArduinoOTA.setPassword("ESP32@OTA*123");
  ArduinoOTA.setPasswordHash("E43A5EF0A6D7C4B5D95ACCDFCD7E8851");

  ArduinoOTA.onStart([]() {
    Serial.println("Se comienza con el update OTA");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("El update finalizó");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progreso: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Error en la autenticación");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Error al comenzar el update");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Error al conectar");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Error al recibir");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Error al finalizar el update");
    }
  });

  ArduinoOTA.begin();

}

/*
 * Reinicia el dispositivo
 */

void deviceReset() {

  delay(3000);
  //ESP.reset(); Analizar que libreria usar.
  delay(5000);

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


  server.begin();
  UpdateFirmware();
  
  SetupOta();
}

void loop() {

  server.handleClient();
  SendVersion();
  delay(10);

}
