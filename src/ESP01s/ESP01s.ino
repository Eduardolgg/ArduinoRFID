/*
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 */


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "Your_wifi_SSID_here"
#define STAPSK  "Your_wifi_password_here"
#endif

const uint32_t baud = 9600;

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

String inputString = "";         // A String to hold incoming data
bool stringComplete = false;     // whether the string is complete

void handleRoot() {
  server.send(200, "text/plain", "ok");
  if (server.arg("command").equals("Disarm"))
    Serial.printf("Disarm\n");
  if (server.arg("command").equals("Armed"))
    Serial.printf("Armed\n");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(baud);
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  WiFiMulti.addAP(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  /*server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });*/

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Wifi module started");
}

void leerComando() {
  while (Serial.available()) {
    // Get the new byte:
    char inChar = (char)Serial.read();    
    // Serial.printf("leerComando: leido %c comando: %s\n", inChar, inputString.c_str());
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
      stringComplete = true;
    else
      // add it to the inputString: I don't want a \n at end.
      inputString += inChar;
  }
}

/**
 * Send the RFID Tag detected to verify in the control unit.
 */
void ejecutarComando(String command) {
  HTTPClient http;
  WiFiClient client;
  String url = "http://192.168.xxx.xxx/json.htm?type=command&param=updateuservariable&idx=18&vname=rfid_id&vtype=2&vvalue=";
  url.concat(command);
  url.trim();

  if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.printf("Waiting for wifi to reconnect");
    while ((WiFiMulti.run() != WL_CONNECTED))
      Serial.print(".");
    Serial.println("");
  }

  Serial.printf("Run command: dest url: %s\n", url.c_str());
  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        Serial.printf("ejecutarComando: Tag %s send success \n", command.c_str());
      else {
        Serial.printf("ejecutarComando: Call error %i\n  response: ", httpCode);
        //String payload = http.getString();
        //Serial.println(payload);
      }
    } else
      Serial.printf("ejecutarComando: [HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str()); 
    http.end();
  } else
    Serial.printf("ejecutarComando: [HTTP} Unable to connect\n");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  leerComando();
  if (stringComplete) {
    ejecutarComando(inputString);
    inputString = "";
    stringComplete = false;
  }
}
