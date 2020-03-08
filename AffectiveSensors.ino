#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

#define BAUD_RATE 57600
// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
#define interrupt 2
// pin setup
#define PIN_EMG A0
#define PIN_GSR A1
#define PIN_PPG D2

// intermediate values
int val_GSR = 0;
int gsr_average = 0;

// only output values
float data_emg;
float data_gsr;
float data_ppg_hr;
float data_ppg_raw;

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 * 5 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0;         // time of last sync()

#define ECHO_TO_SERIAL 1 // echo data to serial port
#define WAIT_TO_START 0  // Wait for serial input in setup()

// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3

// #define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!
// #define bandgap_voltage 1.1      // this is not super guaranteed but its not -too- off


// define the Real Time Clock object
RTC_PCF8523 rtc;

// for the data logging shield, we use digital pin 10 for the SD cs line
#define chipSelect 10

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while (1)
    ;
}

void setup()
{
  Serial.begin(BAUD_RATE);

  // setuprtc();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // attachInterrupt(0, interrupt, RISING); //set interrupt 0,digital port 2

  Serial.println();

  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available())
    ;
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename))
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // leave the loop!
    }
  }

  if (!logfile)
  {
    error("Couldn't create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();
  if (!rtc.begin())
  {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif //ECHO_TO_SERIAL
  }

  logfile.println("millis,stamp,datetime,emg,gsr,ppg");
#if ECHO_TO_SERIAL
  Serial.println("millis,stamp,datetime,emg,gsr,ppg");
#endif //ECHO_TO_SERIAL
}

bool running = true;
int delayCounter = 10;

// the loop routine runs over and over again forever:
void loop()
{
  DateTime now;

  int val_emg = analogRead(PIN_EMG);
  // Convert the analog reading (which goes from 0 - 1023)
  // to a voltage (0 - 5V):
  float voltage_emg = val_emg * (5.0 / 1023.0);
  data_emg = val_emg;
  // print out the value you read:
  // Serial.println(voltage_emg);

  // GSR Code
  long gsr_sum = 0;
  //Average the 10 measurements to remove jittering??
  for (int i = 0; i < 10; i++)
  {
    val_GSR = analogRead(PIN_GSR);
    gsr_sum += val_GSR;
    // sendSerialData();
    delay(10);
  }
  gsr_average = gsr_sum / 10;
  data_gsr = gsr_average;
  // Serial.println(gsr_average);

// TODO: PPG

  now = rtc.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
  // EMG
  logfile.print(data_emg);
  logfile.print(", ");
  // GSR
  logfile.print(data_gsr);
  logfile.print(", ");
  // PPG
  logfile.print(data_ppg_hr);
  logfile.print(data_ppg_raw);
  // logfile.print(", ");
  logfile.println();

#if ECHO_TO_SERIAL
  // Serial.println("rtc_unix_time,millis,stamp,datetime,emg,gsr,ppg");
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(", ");

  Serial.print(millis()); // seconds since 1/1/1970
  Serial.print(", ");

  Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
  Serial.print(", ");
  // EMG
  Serial.print(data_emg);
  Serial.print(", ");
  // GSR
  Serial.print(data_gsr);
  Serial.print(", ");
  // PPG
  Serial.print(data_ppg_hr);
  Serial.print(data_ppg_raw);
  Serial.print(", ");
  Serial.println();

#endif //ECHO_TO_SERIAL

  digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL)
    return;
  syncTime = millis();

  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);
}
