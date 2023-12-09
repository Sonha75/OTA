/*
  

    ESP32 Pin       STM32 MCU      NodeMCU Pin(ESP32 based)
    RXD                    PA9             RXD
    TXD                    PA10            TXD
    Pin4                   BOOT0           D2
    Pin5                   RST             D1
    Vcc                    3.3V            3.3V
    GND                    GND             GND
    En -> 10K -> 3.3V
    TX-SERIAL (PC)                         D6
    RX-SERIAL (PC)                         D7

*/
#include "stm32ota.h"
#include <WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

#include <FS.h>
#include <HTTPClient.h>
#include <Arduino_ESP32_OTA.h>
#include <SoftwareSerial.h>
#include "Arduino_JSON.h"  //Version 6.15.2
#define ARDUINOJSON_USE_LONG_LONG 1

SoftwareSerial Debug;  //For debug only
#define MYPORT_TX 14   //For debug only
#define MYPORT_RX 13   //For debug only

stm32ota STM32(5, 4, 2);  //For use with libray STM32OTA


const char* ssid = "SonHa";  //you ssid
const char* password = "07052003";  //you password
const char* link_Updt = link_Updt = "https://raw.githubusercontent.com/Sonha75/OTA/main/LinkUpdt.txt";
char link_bin[100];
boolean MandatoryUpdate = false;
//----------------------------------------------------------------------------------
const int buttonPin = 9;
const int ledPin = 2;
boolean aux = false;
unsigned long lastTime;
int button = true;

//----------------------------------------------------------------------------------
void wifiConnect() {
  Debug.println("");
  WiFi.disconnect(true);  
  WiFi.mode(WIFI_STA);
  delay(2000);  //wait for stable
  WiFi.begin(ssid, password);
  byte b = 0;
  while (WiFi.status() != WL_CONNECTED && b < 60) {  //time for connect
    b++;
    Debug.print(".");
    delay(500);
  }
  Debug.println("");
  Debug.print("IP:");
  Debug.println(WiFi.localIP());
}
//----------------------------------------------------------------------------------
void checkupdt(boolean all = true) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  WiFiClientSecure client;
  client.setInsecure();  //skip verify
  HTTPClient http;
  http.begin(client, link_Updt); //connect link raw to get data
  int httpCode = http.GET(); //start connection and send GET request
  String s = "";
  s = http.getString();
  http.end();
  s.trim(); //delete space both side

  if (all) {  //boolean
    Debug.println(s);  // //debug  //3
  }


  if (httpCode != HTTP_CODE_OK) {   // check if file found at server
    return;
  }

  StaticJsonDocument<800> doc; //Json document 
  deserializeJson(doc, s); //analyze Json document and store
  strlcpy(link_bin, doc["link"] | "", sizeof(link_bin)); //copy string ['link'] (link bin) to string
  MandatoryUpdate = doc["mandatory"] | false;
  Debug.println(link_bin);  //For debug only  //4
  //Debug.println(MandatoryUpdate);                   //For debug only
}

//----------------------------------------------------------------------------------
void setup() {
  Debug.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);  //For debug only
  Serial.begin(9600, SERIAL_8E1);
  Debug.println("DEBUG SOFTWARESERIAL"); //1
  SPIFFS.begin();  
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  delay(200);
  wifiConnect(); 
  delay(200);
  //STM32.RunMode();
  
  checkupdt();  //2
  Debug.println("END OF INITIALIZATION"); //5
}

void loop() {

  button = digitalRead(buttonPin);
  if (!button) {
    digitalWrite(ledPin, HIGH);
    Debug.println("START UPDATE");  //6
    delay(2000);
    checkupdt(false);  //not prints link_update
    String myString = String(link_bin);
    Debug.println(STM32.otaUpdate(myString));  //update link bin  //7.print link bin
    Debug.println("END OF UPDT");  //9.end of update            //For debug only
  }
  //_______________________________________________________________________

  if (millis() - lastTime > 500) {             //BLINK LED BULTIN
    if (aux) {
      aux = false;
    } else aux = true;
    lastTime = millis();
    digitalWrite(ledPin, aux);
  }
}
