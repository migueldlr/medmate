//Magnet Sensor Test on Arduino Uno (to verify the sensors work period.)

int analogPin = A0;
int magnetVal = 0;
int state = 0;
long state1Timer;

void setup() {
  Serial.begin(115200);
}

void loop() {
  magnetVal = analogRead(analogPin);
  Serial.println(magnetVal);

//  switch(state) {
//    case 0:
//      if(magnetVal >= 195 && magnetVal <= 205) { // get this range when picked up
//        state = 1; // transition!
//      } else {
//        state = 0;
//      }
//     
//      break;
//    case 1: // Start timer
//      state1Timer = millis();
//      state = 2; // instant transition
//    break;
//    
//    case 2: // The medicine is gone!
//      if (!(magnetVal >= 195 && magnetVal <= 205)) {
//          state = 4;
//      } else if (millis() - state1Timer >= 60000) { // wait a whole minute, to wait for medicine to return
//        state = 3;
//      }
//    
//    break;
//
//    case 3: // Alert time
//      if (!(magnetVal >= 195 && magnetVal <= 205)) {
//        state = 4;
//      } else {
//        Serial.println("Alert!");
//        //display alert online
//      }
//    break;
//  
//    case 4: // Record!
//    Serial.println("Record!");
//    // record value
//    state = 0;
//    break;
//  
//   
//  }
//  
//  if (state != 2) { // to slow down the readings, reduce processing I guess...
//      delay(1000);
//  }
  
}
