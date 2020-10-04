String test = String("Hello");
String number = String(0);
String superTest = String(test + number);
String dblTest = String("09");
String dblTest00 = String("00");
String test2 = String(dblTest + ":" + test); 
String onePlaceholder = String(1); // Using 1 as the first number of the output because otherwise it wont save correctly in ThingSpeak (will remove the 00 entries)
int nowHour = 1;
int nowMin = 2;

void setup() {
  Serial.begin(115200);
}

void loop() {
//  Serial.println(test);
//  Serial.println(superTest);
//  Serial.println(test2);
//  Serial.println("HI");
//  medicineAddTime();
//  Serial.println(dblTest.toInt());
//  Serial.println(dblTest00.toInt());
  String currTime = String(onePlaceholder + nowHour + nowMin); // adding the piece of current time together
  String testTime = String(onePlaceholder + String(nowHour) + String(nowMin));
  Serial.println(currTime);
}



void medicineAddTime() {
  String nextHour;
  String nextMin;
  String zeroPlaceholder = String(0);
  String zeroZeroPlaceholder = String("00");
  String onePlaceholder = String(1);
  int medicineTakenHour = 23;
  int medicineTakenMin = 9;
//  Serial.println(medicineTakenHour);
//  Serial.println(medicineTakenMin);

  medicineTakenHour = (medicineTakenHour  + 1) % 24;

  if (medicineTakenHour < 10) {
    if (medicineTakenHour == 0) { 
      nextHour = String("00");
    } else {
      nextHour = String(zeroPlaceholder + medicineTakenHour);
      Serial.println("Next Hour: " + nextHour);
    }
  } else {
    nextHour = String(medicineTakenHour);
  }

  if (medicineTakenMin < 10) {
    if (medicineTakenMin == 0) {
      nextMin = zeroZeroPlaceholder;
    } else {
      nextMin = String(zeroPlaceholder + medicineTakenMin);
      Serial.println("Next Min: " + nextMin);
    }
  } else {
    nextMin = String(medicineTakenMin);
  }
  
  String nextTime = String(onePlaceholder + nextHour + nextMin);
  Serial.println("Next Time: " + nextTime);
}
