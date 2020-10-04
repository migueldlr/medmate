// Read Delay Test
// Results: There is no delay when reading / when a field updates. There is only a 15s delay when uploading.

#include "ThingSpeak.h"
#include <WiFi.h>

// WIFI 
#define SECRET_SSID "FiOS-3VPP9"    // replace MySSID with your WiFi network name
#define SECRET_PASS "eat2359omni564bare"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID 1076743     // replace 0000000 with your channel number
#define SECRET_READ_APIKEY "BOCRGKV103WDJD6Y"

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myReadAPIKey = SECRET_READ_APIKEY;

int ReadyToTake;

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

  ReadyToTake = ThingSpeak.readIntField(myChannelNumber, 2, myReadAPIKey); // The first variable of the logic, need to figure whether to eat or not!
  Serial.println(ReadyToTake);
 
}
