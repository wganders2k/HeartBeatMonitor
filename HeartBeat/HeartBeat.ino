#include <LiquidCrystal.h>
#include <stdlib.h>
const int rs =  4, en = 9, d4 = 8, d5 = 7, d6 = 6, d7 = 5, led = 0, led2 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//test
int sensorHistory[25], peakGap, lastPeakGap, palp, beatCount, counter = 0, dataCounter = 0;
unsigned long peakTime, lastPeakTime;
float bpm, beats[50], temp;
boolean isBeat = false, firstCycle, finger, removed = false;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  lcd.begin(16, 2);
  lcd.print("Collecting Data");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  palp = 0;//counter for number of palpitations recorded
  beatCount = 0;//counter for heartbeats
  firstCycle = true;
  removed = false;
  counter = 0;
  dataCounter = 0;
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println(removed);
  if (removed && finger) {
    setup();
  }
  int sensorValue = analogRead(A0);// read the input on analog pin 0:
  //Serial.println(sensorValue);
  storeData(sensorValue);
  fingerCheck();
  if (/*millis() > 3000 && */finger) { // to prevent bad data from influencing operation TODO: make it based on when the user first puts their finger into the detector

    if (sensorValue > 800 && isBeat == false) {
      
      beatCount++;
      addBeat(float(millis()) / 60000); //to update the bpm value
      isBeat = true;//determines that a beat has begun
      lastPeakTime = peakTime;//to record the last 2 heartbeat timings
      peakTime = millis();//records the beginning of the beat
      lastPeakGap = peakGap;//to store the last 2 heartbeat intervals
      peakGap = peakTime - lastPeakTime;//calculate the time between 2 heartbeats
      
      if (peakGap > 1.3 * lastPeakGap && beatCount >= 5) { //detects if the time between beats was too long TODO: make lastPeakGap an average of the past couple values
        palp++;
        digitalWrite(led2, HIGH);
      }
    }

    //if there is too much time between beats, the program decides that there is no finger and removed is set to true to run setup() once the finger is replaced
    if (millis() > peakTime + 5 * peakGap && beatCount >= 5) {
      finger = false;
      removed = true;
      beatCount = 0;
      lcdPrint(bpm, palp);
    }
    
    if (isBeat) {//if a beat is currently happening
      digitalWrite(led, HIGH);
      if (sensorValue < 800) {//check if the beat is still happening
        isBeat = false;
      }
    }
    else {//turn off leds
      digitalWrite(led, LOW);
      digitalWrite(led2, LOW);
    }
  }

  delay(2);        // delay in between reads for stability
}

//adds the timing information to an array in order to calculate bpm
void addBeat(float currentTime) {

  if (counter > 49) { // checks if the counter will result in an out of bounds exception, if so it is reset to the beginning of the array
    counter = 0;
    firstCycle = false;
  }

  if (firstCycle) { //first cycle since the program started will use the current tally of beats instead of 50
    temp = currentTime - (beats[0]);
    bpm = (counter + 1) / temp;
  }
  else {
    temp = currentTime - (beats[counter]);
    bpm = 50 / temp; // past the first cycle, 50 beats will have been recorded
  }

  beats[counter] = currentTime;
  if (beatCount >= 5) {//does not display the first couple readings as they are very inaccurate
    lcdPrint(bpm, palp);
  }

  counter++;//to iterate through the array
}

//prints relevant information to the lcd display
void lcdPrint(float bpm, float palp) {
  if (finger) {
    lcd.clear();
    lcd.print("BPM: " + String(bpm));
    lcd.setCursor(0, 1);
    lcd.print("Palp: " + String(palp));
  }
  else {
    lcd.clear();
    lcd.print("No finger found");
    lcd.setCursor(0, 1);
    lcd.print("Replace finger");
  }
}

//checks if a finger has been in the range of the sensor
void fingerCheck() {
  if (millis() % 100 == 0 && !finger) {
    finger = true;
    for (int i = 1; i < (sizeof(sensorHistory)) / (sizeof(sensorHistory[0])); i++) {
      if (sensorHistory[i - 1] == sensorHistory[i] && sensorHistory[i] < 100) {
      }
      else {
        finger = false;
      }
      if (finger) {
        Serial.println("finger detected");
      }
      else {
        // Serial.println("no finger detected");
      }

    }
  }
}

//stores the last *25* sensor values detected
void storeData(int sensorValue) {
  if (millis() % 10 == 0) {
    if (dataCounter > (sizeof(sensorHistory) / sizeof(sensorHistory[0]))) {
      dataCounter = 0;
    }
    sensorHistory[dataCounter] = sensorValue;
    dataCounter++;
  }
}
