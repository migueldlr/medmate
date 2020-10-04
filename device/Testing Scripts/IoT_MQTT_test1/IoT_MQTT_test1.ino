
#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>


// WIFI 
#define SECRET_SSID "FiOS-3VPP9"    // replace MySSID with your WiFi network name
#define SECRET_PASS "eat2359omni564bare"  // replace MyPassword with your WiFi password
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)


// JSON
String id; // user ID

// Time Capture
char nowHourChar[30]; //vector used to convert characters --> integers
char nowMinChar[30]; 
char nowDateChar[30]; 

// MQTT
const char* mqtt_server = "mqtt.eclipse.org";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int nowHour; // integer representing the current hour
int nowMin; // integer representing the current minute
int nowDate; 

// Timing
unsigned long putBackTimer = 0; // timer representing how much time you have to put back the medicine before an alert will trigger
 
//Time Library
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Five hours behind in EST? have to check.
const int   daylightOffset_sec = 3600;

//Magnet Setup
int analogPin = 34;
int magnetVal = 0;
int state = 0; // starts default at 0
int state2 = 0;  // starts default at 0 
boolean magPresent;

// Ready To Take Token
boolean ReadyToTake = false; //ready take token, starts false



/* Setup Functions
 *  
 */


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  serializeJsonPretty(doc, Serial);
  ReadyToTake = doc["ready"];
//  id = doc["id"];
  const char* newID = doc["id"];
//  int IDLength = sizeof(newID)/sizeof(newID[0]);
//  memcpy(id, newID, IDLength);
//  Serial.println(IDLength);
//  Serial.println(newID);
  id = newID;
  Serial.println(id);
  Serial.println(ReadyToTake);
}


void reconnectWifi() {
    Serial.print("Attempting to reconnect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(1000);     
    } 
    Serial.println("\nReconnected.");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Stephen Mock";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish("medmate/ESP32Publish", "hello world");
      // ... and resubscribe
      client.subscribe("medmate/ESP32Sender");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  delay(1000);
}




void setup() {  
  Serial.begin(115200);  //Initialize serial
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // setting up time
  putBackTimer = millis(); //initialize it as the time the program starts
}

void loop() {
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    reconnectWifi();
  }
  if (!client.connected()) {
    reconnect();
  }

  
  if (ReadyToTake) { // It is time to take the medicine
    switch (state) {
      case 0:
        magPresent = magnetReadAndAverage(1000);
        
        if (!magPresent) { // get this range when picked up, (magnet is not present)
        state = 1; // transition if magnet is not present
        Serial.println("Transition to State 1");
        delay(2000);
        } else {
         state = 0; // explicity saying nothing changes
        }
        
      break;

      case 1:
        putBackTimer = millis(); // start a 10 second timer that will alert when the user should put back the medicine
        state = 2; // auto transition
        Serial.println("Transition to State 2");
        break;

      case 2:
        while( ((millis() - putBackTimer) < 10 * 1000) && (state == 2) ) { 
          // wait for 10 seconds before an alert, otherwise keep waiting for the medicine to return
          magPresent = magnetReadAndAverage(100);

          if (magPresent) { 
            state = 3; // magnet has been placed back, time to transition !
            Serial.println("Transition to State 3");
          }

          
        }

        if (state == 2) { // if it times out
          publishMQTT(1); // 1 = alert code
          putBackTimer = millis(); // reset the timer
        }
      break;

      case 3:
          publishMQTT(0); // 0 = send time
          state = 0; // resetting
          ReadyToTake = false;
          Serial.println("ALL DONE");
          delay(3000);
      break;
    }
    Serial.print("State1 : ");
    Serial.println(state);
    
  } else { // It is not time to take the medicine
    client.loop();
    switch(state2) {
      case 0:
            magPresent = magnetReadAndAverage(1000);
            
            if(magPresent) { // Alert the user to put the stuff back if the magnet is not present
              state2 = 0; // chill (do nothing if medicine is there)
            } else {
              if(millis() - putBackTimer > 10000) {
                Serial.println("ERROR");
                publishMQTT(1); // send alert
                putBackTimer = millis();
              }
            }
            Serial.print("State2 : ");
            Serial.println(state2);         
      break;      
    }
    
  }
}





/*
 * Helper Functions
 */


// Gets the current time and stores it into two int variables, nowHour and nowMin which are numerical representatives of the hour / time (military time)
 
void getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  // Storing the Hours, Minutes, and Date into various character arrays
  strftime(nowHourChar, 30, "%H", &timeinfo);
  strftime(nowMinChar, 30, "%M", &timeinfo);
  strftime(nowDateChar, 30, "%d", &timeinfo);
  nowHour = atoi(nowHourChar);
  nowMin = atoi(nowMinChar);
  nowDate = atoi(nowDateChar);
  
}


/* Main Functions
 *  The main functions are medicineAddTime(), readFutureTime(), magnetReadAndAverage(), alertUser(), updateDatabase()
 *  
 *  
 */

void publishMQTT(int state) {
   StaticJsonDocument<200> jsonBuffer;
   JsonObject root = jsonBuffer.to<JsonObject>();
   root["id"] = id; // take what is read in, and publish it back to the user
   JsonObject data = root.createNestedObject("alert");

  switch (state) {
    // If the message to send is the normal time of intake 
    case 0:  {
    root["alert"] = false;
    break;
    }
    
    // If the message is an alert
    case 1: {
    root["alert"] = true;
    break;
    }
  }
  getCurrentTime();
  root["hour"] = nowHour;
  root["min"] = nowMin;
  root["date"] = nowDate;
  serializeJson(root, Serial);
  char buffer[256];
  serializeJson(root, buffer);
  client.publish("medmate/ESP32Publish", buffer);
  delay(2000);
}



/*
 * Returns a boolean of whether the device is there or not based on the magnet read values that are averaged with 100 samples.
 * Input is the amount of samples that it takes every time the function runs
 */
boolean magnetReadAndAverage(int numSamples) { 
  int magSum = 0; // sum of all the samples
   for (int i = 0; i < numSamples; i++) { // sampling 100 times before deciding on the magnetVal value
          int magnetValRead = analogRead(analogPin);
          magSum = magSum + magnetValRead;
        }
   magSum = magSum / numSamples; // averaging
   
//   Serial.print("magSum: ");
//   Serial.println(magSum);
   if (magSum >= 1100 && magSum <= 1150) {
    magPresent = false;
   } else {
    magPresent = true; 
   }
   
//   Serial.print("magPresent: ");
//   Serial.println(magPresent);
   return magPresent;
}
