/*
Using the online time server, upload the exact time (as string) when things are being done
*/

#include "ThingSpeak.h"
#include <WiFi.h>
#include "time.h"

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
int medicineTakenHour;
int medicineTakenMin; 

// Timing
unsigned long previousMillis;

//Time Library
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Five hours behind in EST? have to check.
const int   daylightOffset_sec = 3600;


// 24 Hour Passed Token
int Pass24Hour;

// time to eat 
int ReadyToTake; // ready to take the medicine token

void setup() {  
  Serial.begin(115200);  //Initialize serial
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  previousMillis = millis();
 configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // setting up time

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
//    Serial.println(medicineTimeTaken);
  } else {
    //medicineAddTime();
    //medicineTimeTaken = ThingSpeak.readRaw(myChannelNumber, String(String("/fields/") + String(1) + String("/created_at")), myReadAPIKey);
//    medicineTimeTaken = ThingSpeak.readCreatedAt(myChannelNumber, myReadAPIKey); // can read the latest time an entry is entered, but for the whole channel, not a specific field..
    //printLocalTime();
    readFutureTime(); 
    previousMillis = millis();
  }

 
}


void readFutureTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char nowHourChar[30];
  char nowMinChar[30];
  strftime(nowHourChar, 30, "%H", &timeinfo);
  strftime(nowMinChar, 30, "%M", &timeinfo);
  String medicineFutureTime = ThingSpeak.readStringField(myChannelNumber, 1, myReadAPIKey);
  int futureHour = medicineFutureTime.substring(1,3).toInt();
  int futureMin = medicineFutureTime.substring(3).toInt();
  Pass24Hour = ThingSpeak.readIntField(myChannelNumber, 6, myReadAPIKey);
  int nowHour = atoi(nowHourChar);
  int nowMin = atoi(nowMinChar);
//  Serial.println(medicineFutureTime);
//  Serial.println(futureHour);
//  Serial.println(futureMin);
//  Serial.println(nowHour);
//  Serial.println(nowMin);

  if (Pass24Hour == 1) { // Reset this token 00:00 if it has been enabled
    if (nowHour == 0 && nowMin >= 0) { // Its 00:00! --> Update token
      Pass24Hour = 0; // now its time to update 24 hour token
    } else {
      ReadyToTake = 0; // Not time to take it
    }
  } else { // no weird 24Hour edge case
    if (nowHour >= futureHour && nowMin >= futureMin) {
      ReadyToTake = 1; // The hour and the Minute has passed, it is now time to take
    } else {
      ReadyToTake = 0; // Still waiting
    }
    Pass24Hour = 0; // 24 Hour Pass Edge case is 0 still, just explicitly stating here
  }

  int a = ThingSpeak.setField(2, ReadyToTake); // Updating if the time has passed / not and ready to take medicine
  int b = ThingSpeak.setField(6, Pass24Hour); // Updating the Pass24Hour variable
  int c = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(a == 200 & b == 200 & c == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(c));
  } 
}

void medicineAddTime() {
  String nextHour; // Calculting the next time (hour) to take medicine
  String nextMin; // Calculating the next time (minute) to take medicine
  String zeroPlaceholder = String(0); //IDK jank way to add 0 to numeric strings
  String zeroZeroPlaceholder = String("00"); //IDK jank way to add 00 to numeric strings
  String onePlaceholder = String(1); // Using 1 as the first number of the output because otherwise it wont save correctly in ThingSpeak (will remove the 00 entries)
  medicineTakenHour = ThingSpeak.readIntField(myChannelNumber, 4, myReadAPIKey); // When the medicine was consumed Hr
  medicineTakenMin = ThingSpeak.readIntField(myChannelNumber, 5, myReadAPIKey);  // When the medicine was consumed Min
  
//  Serial.println(medicineTakenHour);
//  Serial.println(medicineTakenMin);

  
  medicineTakenHour = (medicineTakenHour  + 1); // Adding 1 hour, using modulus for 24 hour (day)
  if (medicineTakenHour > 24) {
    Pass24Hour = 1; // Set the token saying 24 hours has passed, just to compare the times
    medicineTakenHour = medicineTakenHour % 24; // Making sure to account for 24 hour cycle 
  } else {
    Pass24Hour = 0; // Just set the token as 0, and dont % 24
  }
  
  if (medicineTakenHour < 10) { //In order to format the output when the value is less than 10, with a length 2, 09 vs 9
    if (medicineTakenHour == 0) { 
      nextHour = String("00");  // when 0, output 00
    } else {
      nextHour = String(zeroPlaceholder + medicineTakenHour); // adding a 0 to the single digit value
    }
  } else {
    nextHour = String(medicineTakenHour);
  }

  if (medicineTakenMin < 10) { // do the same for the minutes
    if (medicineTakenMin == 0) {
      nextMin = zeroZeroPlaceholder;
    } else {
      nextMin = String(zeroPlaceholder + medicineTakenMin);
    }
  } else {
    nextMin = String(medicineTakenMin);
  }
  
  String nextTime = String(onePlaceholder + nextHour + nextMin); // Adding a onePlaceholder so that if the time is 00:00, it would not --> 0
  Serial.println("Next Hour: " + nextHour);
  Serial.println("Next Min: " + nextMin);
  Serial.println("Next Time: " + nextTime);
  int a = ThingSpeak.setField(1, nextTime);
  int b = ThingSpeak.setField(6, Pass24Hour);
  int c = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(a == 200 & b == 200 & c == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(c));
  }
}


void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char hour[30];
  char minute[30];
  strftime(hour, 30, "%H", &timeinfo);
  strftime(minute, 30, "%M", &timeinfo);
//  Serial.println(timeWeekDay);
  Serial.println("Sending to ThingSpeak");
  String hourString(hour);
  String minString(minute);
  int a = ThingSpeak.setField(4, hourString);
  int b = ThingSpeak.setField(5, minString);
  int c = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(a == 200 && b == 200 & c == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(c));
  }
  
}
