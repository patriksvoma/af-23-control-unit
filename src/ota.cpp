
#include "bluetooth.h"

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#ifndef STASSID
#define STASSID "AF23_Update"
#define STAPSK "12345678"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
#define OTA_PACKET 0x04


extern Bluetooth bluetooth;
 
namespace ota
{
   void enter(){
    Serial.begin(9600);
  Serial.println("Booting");
  WiFi.mode(WIFI_AP);
  
  WiFi.softAP(ssid, password);
  delay(2000);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    rp2040.restart();
  }

 IPAddress apIP = WiFi.softAPIP();
 
  Serial.print("AP IP Address: ");
  Serial.println(apIP);
    uint8_t ip[4] {apIP[0],apIP[1],apIP[2],apIP[3]};
    bluetooth.sendPacket(OTA_PACKET,ip,4);


  // Port defaults to 2241
  ArduinoOTA.setPort(2241);

  // Hostname defaults to pico-[ChipID]
  // ArduinoOTA.setHostname("mypico");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();



  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  u32_t otaStart = millis();
    while (millis()-otaStart<12000)
    {
        ArduinoOTA.handle();
    }

    //Send callback on succes
    //Send failed
    
   }
}