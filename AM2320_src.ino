#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_AM2320.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Hash.h>

// WiFi credentials
const char* ssid = "ssid";
const char* password = "password";

String host = "http://127.0.0.1:8080";

//Setting of ESP System
int idDevice = 1; //Identity in system
String secretToken = "token"; //Secret token
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// current temperature & humidity, updated in loop()
float t = 0.0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 1 minutes
const long interval = 1000 * 60;

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  am2320.begin();
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void loop() {
  unsigned long currentMillis = millis();
  if (previousMillis == 0 || currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;
    float newT = am2320.readTemperature();
    if (!isnan(newT)) {
      t = newT;
    }
    String privateKey = getPrivateKey(idDevice,secretToken,getSessionKey(idDevice,secretToken));
    HTTPClient http;    //Declare object of class HTTPClient
    String Link;
    String hash = sha1(sha1(String(idDevice)) + sha1(String(secretToken)) + sha1(String(t)) + sha1(privateKey));

    String data = "?id=" + String(idDevice) + "&temperature=" + String(t) + "&signature=" + hash;
    Link = host+"/api/v1/temperature" + data;
    
    http.begin(Link);     //Specify request destination

    int httpCode = http.GET();            //Send the request
    String payload = http.getString();    //Get the response payload
    Serial.println(Link);

    Serial.println(payload);    //Print request response payload
    http.end();  //Close connection
  }
}


String getSessionKey(int idDevice, String password){
   HTTPClient http;    //Declare object of class HTTPClient
    String Link;
    String hash = sha1(sha1(String(idDevice)) + sha1(String(password)));

    String data = "?id=" + String(idDevice) + "&signature=" + hash;
    Link = host+"/api/v1/session" + data;

    http.begin(Link);     //Specify request destination

    int httpCode = http.GET();            //Send the request
    String response = http.getString();    //Get the response payload
    http.end();
    Serial.println("SESSION::"+response);
    return response;
}

String getPrivateKey(int idDevice, String password, String sessionKey){
    HTTPClient http;    //Declare object of class HTTPClient
    String Link;
    String hash = sha1(sha1(String(idDevice)) +sha1(String(sessionKey))+ sha1(String(password)));

    String data = "?id=" + String(idDevice)+"&sessionKey=" + sessionKey + "&signature=" + hash;
    Link = host+"/api/v1/privateKey" + data;

    http.begin(Link);     //Specify request destination

    int httpCode = http.GET();            //Send the request
    String response = http.getString();    //Get the response payload
    http.end();
    Serial.println("PRIVATE::"+response);
   return response;
}
