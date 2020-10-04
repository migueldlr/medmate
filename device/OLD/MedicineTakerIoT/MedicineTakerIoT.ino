
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

//  String Constants
String zeroPlaceholder = String(0); //IDK jank way to add 0 to numeric strings
String zeroZeroPlaceholder = String("00"); //IDK jank way to add 00 to numeric strings
String onePlaceholder = String(1); // Using 1 as the first number of the output because otherwise it wont save correctly in ThingSpeak (will remove the 00 entries)

// Time Capture
char nowHourChar[30]; //vector used to convert characters --> integers
char nowMinChar[30]; 
int nowHour; // integer representing the current hour
int nowMin; // integer representing the current minute
int futureHour; 

// Timing
unsigned long thingSpeak15SecDelay = 0; 
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
long state1Timer;

// 24 Hour Passed Token, Field 3
int Pass24Hour;

// Ready To Take Token, Field 2
int ReadyToTake; // ready to take the medicine token

void setup() {  
  Serial.begin(115200);  //Initialize serial
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // setting up time
  thingSpeak15SecDelay = millis(); //initialize it as the time the program starts
  putBackTimer = millis(); //initialize it as the time the program starts
}

void loop() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(1000);     
    } 
    Serial.println("\nConnected.");
  }
  
  ReadyToTake = ThingSpeak.readIntField(myChannelNumber, 2, myReadAPIKey); // The first variable of the logic, need to figure whether to eat or not! 1 = true, 0 = false

  if (ReadyToTake == 1) { // It is time to take the medicine
    switch (state) {
      case 0:
        magPresent = magnetReadAndAverage(1000);
        
        if (!magPresent) { // get this range when picked up, (magnet is not present)
        state = 1; // transition if magnet is not present
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
        while( ((millis() - putBackTimer) < 10 * 1000) && (state == 2) ) { 
          // wait for 10 seconds before an alert, otherwise keep waiting for the medicine to return
          magPresent = magnetReadAndAverage(100);
          Serial.println("In state 2");

          if (magPresent) { 
            state = 3; // magnet has been placed back, time to transition !
            Serial.println("Transition to State 3");
            delay(1000);
          }

          
        }

        if (state == 2) { // if it times out
          alertUserPutback(); // send an alert to putback the medicine
          putBackTimer = millis(); // reset the timer
        }
      break;

      case 3:
        if (millis() - thingSpeak15SecDelay > 15000) { // only update if the corresponding 15 sec has passed.
          updateDatabase();
          state = 0; // resetting
        }
      break;
    }
//    Serial.print("State : ");
//    Serial.println(state);
    
  } else { // It is not time to take the medicine
  
    switch(state2) {
      case 0:
        if ((millis() - thingSpeak15SecDelay) > 15000) { // the requirement to do any comparisons... is to see first if the 15 sec delay has passed
            magPresent = magnetReadAndAverage(1000);
            
            if(magPresent) { // Alert the user to put the stuff back if the magnet is not present
              state2 = 1; // proceeding as normal to read
              Serial.println("Transition to State2: 2");
              delay(1000);
            } else {
              if(millis() - putBackTimer > 20000) {
                alertUserPutback();
                putBackTimer = millis();
              }
            }
            
          }          
      break;

      case 1:
        readFutureTime();
        thingSpeak15SecDelay = millis(); // setting the 15sec delay again
        state2 = 0; // resetting the state
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
  strftime(nowHourChar, 30, "%H", &timeinfo);
  strftime(nowMinChar, 30, "%M", &timeinfo);
  nowHour = atoi(nowHourChar);
  nowMin = atoi(nowMinChar);
}

// This function works to convert a given hour/minutes into a string, that is 5 long. It returns the formatted "string"
// String = XYZAB: X always = 1, as a place holder. YZ = the Hours, and can represent 00. AB = the minutes, and can represent 00.

String convertTime(int hours, int minutes) {
  String hourString;
  String minString;
  if (hours < 10) { //In order to format the output when the value is less than 10, with a length 2, 09 vs 9
    if (hours == 0) { 
      hourString = zeroZeroPlaceholder;  // when 0, output 00
    } else {
      hourString = String(zeroPlaceholder + hours); // adding a 0 to the single digit value
    }
  } else {
    hourString = String(hours);
  }

  if (minutes < 10) { // do the same for the minutes
    if (minutes == 0) {
      minString = zeroZeroPlaceholder;
    } else {
      minString = String(zeroPlaceholder + minutes);
    }
  } else {
    minString = String(minutes);
  }

  String combinedString = String(onePlaceholder + hourString + minString);
  return combinedString;
}



/* Main Functions
 *  The main functions are medicineAddTime(), readFutureTime(), magnetReadAndAverage(), alertUser(), updateDatabase()
 *  
 *  
 */

// This function reads the current time --> Then calculates the next time the medicine should be taken.
// It accounts for the over "24 hour" edgecase, and sets the Pass24Hour Token if this is to occur, and updates it.

void medicineAddTime() {
  String nextHour; // Calculting the next time (hour) to take medicine
  String nextMin; // Calculating the next time (minute) to take medicine
  
  getCurrentTime(); // update nowHour, nowMin
  
//  Serial.print("nowHour: ");
//  Serial.println(nowHour);
//  Serial.print("nowMin: ");
//  Serial.println(nowMin);  

  
  futureHour = (nowHour + 4); // Adding 1 hour, using modulus for 24 hour (day)
  if (futureHour > 24) {
    Pass24Hour = 1; // Set the token saying 24 hours has passed, just to compare the times
    futureHour = futureHour % 24; // Making sure to account for 24 hour cycle 
  } else {
    Pass24Hour = 0; // Just set the  token as 0, and dont % 24
  }

  
  String currTime = convertTime(nowHour, nowMin);
  String nextTime = convertTime(futureHour, nowMin);
//  String nextTime = String(onePlaceholder + nextHour + nextMin); // Adding a onePlaceholder so that if the time is 00:00, it would not --> 0
//  String currTime = String(onePlaceholder + nowHour + nowMin); // adding the piece of current time together
  
  Serial.print("currTime: ");
  Serial.println(currTime );
  Serial.print("nextTime: ");
  Serial.println(nextTime );
  Serial.print("Pass24Hour: ");
  Serial.println(Pass24Hour);

  ReadyToTake = 0; // No longer time to take!
  
  int a = ThingSpeak.setField(1, nextTime); // Update the next time the medicine should be taking
  int b = ThingSpeak.setField(2, ReadyToTake); // Signify that the medicine is no longer ready to take
  int c = ThingSpeak.setField(3, Pass24Hour); // Update a token, if 24 hours has passed or not
  int d = ThingSpeak.setField(4, currTime); // Updating the time of intake
  int e = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if(a == 200 & b == 200 & c == 200 & d == 200 & e == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(c));
  }

}



// Reads the local time, and then also reads the future time when medicine should be taken 
// If local time passes future time, then... change the "ReadyToTake" token 
// Tries to account for edge cases where it goes past midnight

void readFutureTime() {
  getCurrentTime();
  String medicineFutureTime = ThingSpeak.readStringField(myChannelNumber, 1, myReadAPIKey);
  int futureHour = medicineFutureTime.substring(1,3).toInt();
  int futureMin = medicineFutureTime.substring(3).toInt();
  Pass24Hour = ThingSpeak.readIntField(myChannelNumber, 3, myReadAPIKey);

  Serial.print("Future Time: ");
  Serial.print(futureHour);
  Serial.print(":");
  Serial.println(futureMin);
  
  Serial.print("Current Time: ");
  Serial.print(nowHour);
  Serial.print(":");
  Serial.println(nowMin);
  if (Pass24Hour == 1) { // Reset this token 00:00 if it has been enabled
    if (nowHour == 0 && nowMin >= 0) { // Its at least 00:00! --> Update token
      Pass24Hour = 0; // now its time to update 24 hour token
    } else {
      ReadyToTake = 0; // Not time to take it
    }
  } else { // no weird 24Hour edge case
    if (nowHour >= futureHour) {
      if (nowHour > futureHour) {
        ReadyToTake = 1; // The hour and the Minute has passed, it is now time to take
        alertUserEat(); // time for the user to eat now.
      } else if (nowMin >= futureMin) {
        ReadyToTake = 1;
        alertUserEat(); // time for the user to eat now.
      } else { // The hour might be equal, but the minutes are still less
        ReadyToTake = 0; // Still waiting, just explicity signifying this
      }
    } else {
      ReadyToTake = 0; // Still waiting, explicity signifying this.
    }
    Pass24Hour = 0; // 24 Hour Pass Edge case is 0 still, just explicitly stating here
  }

  Serial.print("ReadyToTake: ");
  Serial.println(ReadyToTake);
  Serial.print("Pass24Hour: ");
  Serial.println(Pass24Hour);
  
  int a = ThingSpeak.setField(2, ReadyToTake); // Updating if the time has passed / not and ready to take medicine
  int b = ThingSpeak.setField(3, Pass24Hour); // Updating the Pass24Hour variable
  int c = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(a == 200 & b == 200 & c == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(c));
  } 
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

// Send an email to tell the people to put the medicine back as a reminder

void alertUserPutback() { 
  Serial.println("Alert, Please Return the Medicine");
  delay(1000);
}

void alertUserEat() {
  Serial.println("Time to Eat the Medicine");
  delay(1000);
}

void updateDatabase() {
  medicineAddTime();
  thingSpeak15SecDelay = millis(); // timer start
}
