//Stephen Mock
// 10-2-20
// Full Code for Pill Dispenser (Not Holder)

#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Adafruit_VCNL4010.h"
#include <Servo.h>

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

//Magnet Setup
int state = 0; // starts default at 0
int state2 = 0;  // starts default at 0 
boolean medPresent;

// Servo
int currPos = 45;
Servo myservo;
int amount = 0;

// USER INFO
boolean readyToTake = false; //ready take token, default as TRUE... unless specified on startup
boolean prescription = false; // by default... this should be false
String userID = "null"; // user ID
String prescriptionID;




/* Setup Functions
 *  
 */




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
  amount = doc["amount"];
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
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object
  servoMove(90);
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
        dispense(amount);
        state = 1;
      break;

      case 1:
        while(state == 1) { 
          // wait for 10 seconds before an alert, otherwise keep waiting for the medicine to return
          sensorReadAndAverage(10);

          if (!medPresent) { 
            state = 2; // medicine has been taken, time to transition !
            Serial.println("Transition to State 2");
          }
      break;

      case 2:
          eventSend(); // success.. send a positive event to the server
          state = 0; // resetting
          readyToTake = false;
          Serial.println("ALL DONE");
          delay(2000);
      break;
    }
    Serial.print("State1 : ");
    Serial.println(state);
    
    }
  } else { // It is not time to take the medicine
      client.loop(); // Just wait until it is time to dispense again... nothing else to do
      Serial.println("WAITING...");
  }
}



/*
 * Helper Functions
 */

// Dispenses a set amount of pills
void dispense(int numPills) {
  for (int i = 0; i < numPills; i++) {
    servoMove(90);
    delay(1000);
    servoMove(45);
    delay(2000);
  }
  Serial.print(numPills);
  Serial.println(" dispensed");
}

// Servo Function that helps to smooth the process, and make sure that it doesnt move too quickly
void servoMove(int angle) {

 if (angle > currPos) {
    for ( ; currPos <=  angle; currPos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
          Serial.println(currPos);
          myservo.write(currPos); // Servo 0 (X ROT)
      delay(30);                       // waits 15ms for the servo to reach the position
    }
 } else {
    for ( ; currPos >=  angle; currPos -= 1) { // goes from 0 degrees to 180 degrees
           myservo.write(currPos); // Servo 0 (X ROT)
           delay(30);                       // waits 15ms for the servo to reach the position
   }
 }
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
   
   if (proximitySum < 10000) { // based on the threshhold of the sensor, general testing
    medPresent = false;
   } else {
    medPresent = true; 
   }
   Serial.print("medPresent?: ");
   Serial.println(medPresent);
}
