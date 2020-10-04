/*  
  Description: Writes a value to a channel on ThingSpeak every 20 seconds.
  
  Hardware: ESP32 based boards
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires installation of EPS32 core. See https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md for details. 
  - Select the target hardware from the Tools->Board menu
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2018, The MathWorks, Inc.
*/

#include "ThingSpeak.h"
#include <WiFi.h>

// WIFI 
#define SECRET_SSID "FiOS-3VPP9"    // replace MySSID with your WiFi network name
#define SECRET_PASS "eat2359omni564bare"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID 1076743    // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "2ZXRN1A7U6EWUIX9"   // replace XYZ with your channel write API Key
#define SECRET_READ_APIKEY "BOCRGKV103WDJD6Y"

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

// Data Capture
int medicineTaken; // 0 = false, 1 = true
String medicineTimeTaken; 

// Timing
unsigned long previousMillis;

void setup() {  
  Serial.begin(115200);  //Initialize serial
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  previousMillis = millis();
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  if (millis() - previousMillis < 15000) {
    Serial.println(medicineTimeTaken);
  } else {
    //medicineTaken = ThingSpeak.readIntField(myChannelNumber, 1, myReadAPIKey);    
    //medicineTimeTaken = ThingSpeak.readRaw(myChannelNumber, String(String("/fields/") + String(1) + String("/created_at")), myReadAPIKey);
    medicineTimeTaken = ThingSpeak.readCreatedAt(myChannelNumber, myReadAPIKey); // can read the latest time an entry is entered, but for the whole channel, not a specific field..
    previousMillis = millis();
  }
 
}
