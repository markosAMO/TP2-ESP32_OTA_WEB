#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include "QueryLib.h"
const char* ssid = "ESP32_AP";
const char* password = "123456789";


/*
 * Login menu HTML
 */

const char* loginIndex =
 "<form name='login'>"
    "<table width='20%' bgcolor='cccccc' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Usuario:</td>"
             "<td><input type='text' size=25 name='user'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Contraseña:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"

"<script>"
    "function check(form)"
    "{"
    "if(form.user.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Index menu HTML
 */

const char* serverIndex =
"<script src='/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

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

/*
 * Se configura el ESP32 como Access Point
 */
void OtaIni(){
Serial.begin(115200);
  delay(10);
  Serial.print("Seteando WiFi en modo Access Point");
  WiFi.mode(WIFI_AP);
  while(!WiFi.softAP(ssid, password))
  {
   Serial.println(".");
    delay(100);
  }

  /*
   * Se incluye seguridad MD5 y manejo de errores al hacer un update
   */

  ArduinoOTA.setPassword("ESP32@OTA*123");
  ArduinoOTA.setPasswordHash("E43A5EF0A6D7C4B5D95ACCDFCD7E8851");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Se comienza con el update del " + type);
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

  Serial.print("Iniciado AP ");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  /*
   * Manejo del endpoint '/' para formulario de login
   */

  server.on("/jquery.min.js", HTTP_GET, onJavaScript);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });

  /*
   * Manejo del endpoint '/serverindex' para el menu de opciones con el boton de update
   */

  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  /*
   * Manejo del endpoint '/update' para subir el archivo
   */

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {

    HTTPUpload & upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
}

void setup(void) {

  OtaIni();

}

/*
 * Establece conexión WiFi
 */

void Connect_WiFi() {

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

}

void loop(void) {

  server.handleClient();
  ArduinoOTA.handle();
  delay(1);
  
}