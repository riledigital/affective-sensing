#include "RTClib.h"

RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// pin setup
const int PIN_EMG = A0;
const int PIN_GSR = A1;
//const int PIN_PPG = D2;

// intermediate values
int val_GSR = 0;
int gsr_average = 0;

// only output values
float data_emg;
float data_gsr;
float data_ppg_hr;
float data_ppg_raw;

void setup() {
  Serial.begin(57600);
  
  setupRTC();

//  attachInterrupt(0, interrupt, RISING);//set interrupt 0,digital port 2
}

bool running = true;
int delayCounter = 10;

// the loop routine runs over and over again forever:
void loop() {
//  loopRTC();
//  ppg_level = digitalRead(2);
  DateTime now = rtc.now();
  
  if (running) {
  int val_emg = analogRead(PIN_EMG);
  // Convert the analog reading (which goes from 0 - 1023) 
  // to a voltage (0 - 5V):
  float voltage_emg = val_emg * (5.0 / 1023.0);
  // print out the value you read:
  Serial.println(voltage_emg);

  // GSR Code
  long gsr_sum = 0;
  //Average the 10 measurements to remove jittering??
  for (int i = 0; i < 10; i++)
  {
    val_GSR = analogRead(PIN_GSR);
    gsr_sum += val_GSR;
    sendSerialData();
    delay(10);
  }
  gsr_average = gsr_sum / 10;
  Serial.println(gsr_average);
  } else {
    // Paused, skip action cycle and don't actually run
    // delayCounter = delayCounter - 1;
  }
}

void  nonBlockDelay(int ms) {

}

void sendSerialData()
{
    DateTime now = rtc.now();
  // time
//  Serial.print(millis());
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
//    Serial.println();
  Serial.print(",");
  // EMG
  Serial.print(data_emg);
  Serial.print(",");
  // GSR
  Serial.print(data_gsr);
//  // PPG
//  Serial.print(data_ppg_hr);
//  Serial.print(data_ppg_raw);
}
