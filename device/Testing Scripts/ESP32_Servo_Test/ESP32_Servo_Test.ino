#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
int currPos = 0;

void setup() {
  Serial.begin(115200);
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object
}

void loop() {
    servoMove(90);
    delay(5000);
    servoMove(45);
    delay(5000);
}



void servoMove(int angle) {
Serial.println("servo move");
Serial.println(angle);

 if (angle > currPos) {
  Serial.println("YES WE ARE HERE");
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
