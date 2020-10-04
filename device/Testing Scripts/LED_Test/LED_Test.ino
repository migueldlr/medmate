// the number of the LED pin
const int ledPin1 = 25;  // 16 corresponds to GPIO16
const int ledPin2 = 33; // 17 corresponds to GPIO17
const int ledPin3 = 32;  // 5 corresponds to GPIO5

// setting PWM properties
const int freq = 5000;
const int ledChannel1 = 0;
const int ledChannel2 = 1;
const int ledChannel3 = 2;

const int resolution = 8;
 
void setup(){
  // configure LED PWM functionalitites
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin1, ledChannel1);
  ledcAttachPin(ledPin2, ledChannel2);
  ledcAttachPin(ledPin3, ledChannel3);
}
 
void loop(){
  ledcWrite(ledChannel1, 123);
  ledcWrite(ledChannel2, 123);
  ledcWrite(ledChannel3, 123);
  //delay(100);
//  // increase the LED brightness
//  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
//    // changing the LED brightness with PWM
//    ledcWrite(led1Channel, dutyCycle);
//    delay(15);
//  }
//
//  // decrease the LED brightness
//  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
//    // changing the LED brightness with PWM
//    ledcWrite(ledChannel, dutyCycle);   
//    delay(15);
//  }
}
