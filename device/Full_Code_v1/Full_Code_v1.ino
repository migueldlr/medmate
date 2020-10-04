//Stephen Mock
// 10-2-20
// Full Code for Pill Holder (not Dispenser)


#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Adafruit_VCNL4010.h"

// Sensor Setup
Adafruit_VCNL4010 vcnl;

// WIFI 
#define SECRET_SSID "FiOS-3VPP9"    // replace MySSID with your WiFi network name
#define SECRET_PASS "eat2359omni564bare"  // replace MyPassword with your WiFi password
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)


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

//State Setup
int state = 0; // starts default at 0
int state2 = 0;  // starts default at 0 
boolean medPresent;

// LED Setup
const int ledPin1 = 32;  // RED
const int ledPin2 = 33; // GREEN
const int ledPin3 = 25;  // BLUE

const int freq = 5000;
const int ledChannel1 = 0;
const int ledChannel2 = 1;
const int ledChannel3 = 2;
const int resolution = 8;


// USER INFO
boolean readyToTake = false; //ready take token, default as TRUE... unless specified on startup
boolean prescription = false; // by default... this should be false
String userID = "null"; // user ID
String prescriptionID;




/* Setup Functions
 *  
 */

void setup_LED() {
  // configure LED PWM functionalitites
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin1, ledChannel1);
  ledcAttachPin(ledPin2, ledChannel2);
  ledcAttachPin(ledPin3, ledChannel3);
}

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


/*
 * Wakeup Function
 */

void startUpMsg() {
   StaticJsonDocument<200> jsonBuffer;
   JsonObject root = jsonBuffer.to<JsonObject>();
   root["deviceId"] = "device1"; // take what is read in, and publish it back to the user
   root["type"] = "register";
   serializeJson(root, Serial);
   char buffer[256];
   serializeJson(root, buffer);
   client.publish("medmate/dev/ESP32Test/d2s", buffer);
   Serial.println("STARTUP MESSAGE");
   delay(5000);
}





void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  serializeJsonPretty(doc, Serial);

  /* There are 5 fields that you can accept:
   *  type: the type of message being sent
   *  userID: the userID to have stored onto this device
   *  prescription: a boolean representing if the device should go into "prescription mode" or not
   *  prescriptionID: an ID associated with a specific prescription
   *  readyToTake: a boolean telling the device if it is time for a user to take the medicine
  */
  
  if (userID.equals("null")) { // If there is no value to the userID... then assign it at the startup, and then this should be the same for the user for all calls.
      const char* newID = doc["userID"];
      userID = newID;
  }
  
  prescription = doc["prescription"];
  const char* newPrescriptionID = doc["prescriptionID"];
  prescriptionID = newPrescriptionID;
  readyToTake = doc["readyToTake"];
  Serial.print("READYTOTAKE?");
  Serial.println(readyToTake);
  Serial.println(" CALL BACK !!!");
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
      client.subscribe("medmate/dev/ESP32Test/s2d"); // subscribe to the server --> device network
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
  if (! vcnl.begin()){ //Initializing sensor
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.println("Found VCNL4010");
  reconnect();
  startUpMsg();
  setup_LED();

}

void loop() {
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    reconnectWifi();
  }
  if (!client.connected()) {
    reconnect();
  }

  
  if (readyToTake) { // It is time to take the medicine
    switch (state) {
      case 0:
        setColor(0,255,0);
        sensorReadAndAverage(10);
        
        if (!medPresent) { // get this range when picked up, (medicine is not present)
        state = 1; // transition if medicine is not present
        Serial.println("Transition to State 1");
        delay(1000);
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
        while( ((millis() - putBackTimer) < 5 * 1000) && (state == 2) ) { 
          // wait for x seconds before an alert, otherwise keep waiting for the medicine to return
          sensorReadAndAverage(10);
          setColor(255,255,0); // Outputs Yellow


          if (medPresent) { 
            state = 3; // medicine has been placed back, time to transition !
            Serial.println("Transition to State 3");
          }

          
        }

        if (state == 2) { // if it times out
          setColor(255,0,0); // output red
          alertServer(); // times out... send generic message to server! tell them to put back the medicine!
          putBackTimer = millis(); // reset the timer
        }
      break;

      case 3:
          eventSend(); // success.. send a positive event to the server
          state = 0; // resetting
          if (prescription) { // if its a prescription... it will go on a timer when you should be taking/ not taking
            readyToTake = false;
          } else {
            readyToTake = true; // if its just tracking whenever medicine is used in general... then prescription dont matter.
          }
          Serial.println("ALL DONE");
          delay(2000);
      break;
    }
    Serial.print("State1 : ");
    Serial.println(state);
    
  } else { // It is not time to take the medicine
    client.loop();
    switch(state2) {
      case 0:
            setColor(153,0,153); // Output Purple
            sensorReadAndAverage(10);
            
            if(medPresent) { // Alert the user to put the stuff back if the magnet is not present
              state2 = 0; // chill (do nothing if medicine is there)
              putBackTimer = millis(); //Reset the timer, becauses the device IS there
            } else {
              if(millis() - putBackTimer > 5 * 1000) {
                Serial.println("ERROR");
                setColor(255,0,0); // output red
                alertServer(); // send alert
                putBackTimer = millis(); // Reset the timer
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
 
// Sets the color of Multicolor LED in 1 move
 void setColor(int red, int green, int blue) 
  {
    ledcWrite(ledChannel1, red);
    ledcWrite(ledChannel2, green);
    ledcWrite(ledChannel3, blue);
  }



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


/*
 *  This function is when a normal event occurs. This means the behavior used the device as intended, and there is no reason to alert or have a change in behavior.
 */
 
void eventSend() {
  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["userID"] = userID; // take what is read in, and publish it back to the user
  root["type"] = "s_event";
  root["prescription"] = prescription;
  root["prescriptionID"] = prescriptionID;
  getCurrentTime();
  root["date"] = nowDate;
  root["hour"] = nowHour;
  root["min"] = nowMin;
  serializeJson(root, Serial);
  char buffer[256];
  serializeJson(root, buffer);
  client.publish("medmate/dev/ESP32Test/d2s", buffer);
  delay(2000);
}


/*
 *  This function is when an alert occurs. The errors are all the same, so the type is only "alert" for now. 
 */

void alertServer() {
  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["userID"] = userID; // tell the device which user the alert is tied to (stored on startup)
  root["type"] = "alert";
  getCurrentTime();
  root["date"] = nowDate;
  root["hour"] = nowHour;
  root["min"] = nowMin;
  serializeJson(root, Serial);
  char buffer[256];
  serializeJson(root, buffer);
  client.publish("medmate/dev/ESP32Test/d2s", buffer);
  delay(2000);
}


/*
 * Returns a boolean of whether the device is there or not based on the magnet read values that are averaged with 100 samples.
 * Input is the amount of samples that it takes every time the function runs
 */
void sensorReadAndAverage(int numSamples) { 
  int proximitySum = 0; // sum of all the samples
   for (int i = 0; i < numSamples; i++) { // sampling 100 times before deciding on the magnetVal value
          int proximityRead = vcnl.readProximity();
          proximitySum = proximitySum + proximityRead;
        }
   proximitySum = proximitySum / numSamples; // averaging
   
   if (proximitySum < 3500) { // based on the threshhold of the sensor, general testing
    medPresent = false;
   } else {
    medPresent = true; 
   }
   Serial.print("medPresent?: ");
   Serial.println(medPresent);
}
