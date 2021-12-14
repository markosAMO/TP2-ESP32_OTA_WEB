#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
/*
 * usuario y contraseña par ala conexión al ESP en modo AP
 */
const char* ssid = "ESP32_AP";
const char* password = "123456789";
const char* version = "3.0.0";
/*
 * Declaramos objeto de la libreria WebServer
 */
 
WebServer server(80);
/*
 * función encargada de realizar el upload
 */
void UpdateFile(){
  WiFiClient client;
  /*
   *Se permiten las redirecciones enviadas en el headder para la librería httpUpdate
   */
  httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  /*
   *Se configura la librería para que la actualización del firmware no reinicie el ESP
   */
  httpUpdate.rebootOnUpdate(false);
  /*
   *incia la carga del archivo y si hay un error en la subida o conexión se imprime el error
   */
  
  t_httpUpdate_return ret = httpUpdate.update(client, "http://192.168.4.2:5000/display/firmware.bin"); 
  
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

void SetupServer() { 
  /*
   * Manejo del endpoint '/update' para subir el archivo
   */
  server.on("/version", HTTP_GET, [](){
    server.sendHeader("Connection", "close");
    server.send(200,"text/plain",version);
  });

  /*
   * Manejo del endpoint '/update' para subir el archivo
   */
  server.on("/update", HTTP_GET, []() {
      UpdateFile();
      Serial.println();
      server.send(200, "text/plain", String("update success"));
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

}