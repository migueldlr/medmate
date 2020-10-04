//Magnet Sensor Test on the ESP32 (different than arduino)
// The range when the magnet is not there is 
//1100 - 1120
// SAMPLING AND AVERAGING IS VERY IMPORTANT

const int analogPin = 34;
int magnetVal = 0;
int state = 0;
long state1Timer;

void setup() {
  Serial.begin(115200);
}

void loop() {
magnetVal = magnetReadAndAverage(1000);
Serial.println(magnetVal);

  switch(state) {
    case 0:
      if(magnetVal >= 1100 && magnetVal <= 1120) { // get this range when picked up
        state = 1; // transition!
      } else {
        state = 0;
      }
     
      break;
    case 1: // Start timer
      state1Timer = millis();
      state = 2; // instant transition
    break;
    
    case 2: // The medicine is gone!
      if (!(magnetVal >= 1100 && magnetVal <= 1120)) {
          state = 4;
      } else if (millis() - state1Timer >= 60000) { // wait a whole minute, to wait for medicine to return
        state = 3;
      }
    
    break;

    case 3: // Alert time
      if (!(magnetVal >= 1100 && magnetVal <= 1120)) {
        state = 4;
      } else {
        Serial.println("Alert!");
        //display alert online
      }
    break;
  
    case 4: // Record!
    Serial.println("Record!");
    // record value
    state = 0;
    break;
  
   
  }
  
  if (state != 2) { // to slow down the readings, reduce processing I guess...
      delay(1000);
  }

  Serial.println(state);
  
}


int magnetReadAndAverage(int numSamples) { 
  int magSum = 0; // sum of all the samples
   for (int i = 0; i < numSamples; i++) { // sampling 100 times before deciding on the magnetVal value
          int magnetValRead = analogRead(analogPin);
          magSum = magSum + magnetValRead;
        }
   magSum = magSum / numSamples; // averaging
   return magSum;
}
