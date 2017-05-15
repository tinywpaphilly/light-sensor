#include <RTClib.h>
#include <Wire.h>
#include <SD.h>
// Note to user:   Look for CHANGE ME and alter if necessary depending on your sensor

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  100 // CHANGE ME:  mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!

#define SYNC_INTERVAL 10000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// the digital pins that connect to the LEDs
// for debugging
#define redLEDpin 2
#define greenLEDpin 3
#define blueLEDpin 4

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
 const int chipSelect = 10;

// the logging file
 File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println();
  
  #if WAIT_TO_START
    Serial.println("Type any character to start");
    while (!Serial.available());
  #endif //WAIT_TO_START

  // initialize the SD card
    Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
   pinMode(chipSelect, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
  #if ECHO_TO_SERIAL
    Serial.println("RTC failed");
# endif  //ECHO_TO_SERIAL
  }
  logfile.println("millis,stamp,datetime,temp");    
  #if ECHO_TO_SERIAL
  Serial.println("millis,stamp,datetime,temp");
  #endif //ECHO_TO_SERIAL
 
  // If you want to set the aref to something other than 5v
  analogReference(DEFAULT);
}

void loop(void)
{

  float counts1;
  float counts2;
  float counts3;
  float counts4;
  int photoDiode1 = 3;
  int photoDiode2 = 2;
  int photoDiode3 = 1;
  int photoDiode4 = 0;
  int levelChange1;
  int levelChange2;
  int levelChange3;
  int levelChange4;
  int timeAvg = 3000;  //1000 per sec
  int countsAvg1 = 0;
  int countsAvg2 = 0;
  int countsAvg3 = 0;
  int countsAvg4 = 0;  
  double countsSum1 = 0;
  double countsSum2 = 0;
  double countsSum3 = 0;
  double countsSum4 = 0;
  int sensor1 = 0;
  int sensor2 = 0;
  int sensor3 = 0;
  int sensor4 = 0;
  
  int logData = 0;

  DateTime now;

  int totalMeasurements = timeAvg/LOG_INTERVAL;

  for (int c = 0; c < totalMeasurements; c++)
    {
      counts1 = analogRead(photoDiode1); //CHANGE ME for sensor pin name
      counts2 = analogRead(photoDiode2); //CHANGE ME for sensor pin name
      counts3 = analogRead(photoDiode3); //CHANGE ME for sensor pin name
      counts4 = analogRead(photoDiode4); //CHANGE ME for sensor pin name
      countsSum1 = countsSum1 + counts1;
      countsSum2 = countsSum2 + counts2;
      countsSum3 = countsSum3 + counts3;
      countsSum4 = countsSum4 + counts4;
      delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
    }

  countsAvg1 = countsSum1/totalMeasurements;
  countsAvg2 = countsSum2/totalMeasurements;
  countsAvg3 = countsSum3/totalMeasurements;
  countsAvg4 = countsSum4/totalMeasurements;

  delay(100);

  counts1 = analogRead(photoDiode1);
  counts2 = analogRead(photoDiode2);
  counts3 = analogRead(photoDiode3);
  counts4 = analogRead(photoDiode4);

  levelChange1 = (countsAvg1*0.25);
  levelChange2 = (countsAvg2*0.25);
  levelChange3 = (countsAvg3*0.25);
  levelChange4 = (countsAvg4*0.25);


  //Serial.print(countsSum1);
  //Serial.print(" ");
  //Serial.print(countsSum2);
  //Serial.print(" ");
  //Serial.print(countsSum3);
  //Serial.print(" ");
  //Serial.print(countsSum4);
  //Serial.print(" ");
  
  Serial.print(countsAvg1);
  Serial.print(" ");
  Serial.print(countsAvg2);
  Serial.print(" ");
  Serial.print(countsAvg3);
  Serial.print(" ");
  Serial.print(countsAvg4);
  Serial.print(" ");
  Serial.print(levelChange1);
  Serial.print(" ");
  Serial.print(levelChange2);
  Serial.print(" ");
  Serial.print(levelChange3);
  Serial.print(" ");
  Serial.print(levelChange4);
  Serial.println();

  if (counts1 < countsAvg1-levelChange1)
  {  
      Serial.print("Sensor 1: Sitting, ");
      Serial.println(counts1);
      sensor1 = 1;
      logData = 1;
  }

   if (counts1 > countsAvg1 + levelChange1)
  {  
      Serial.print("Sensor 1: Got up, ");
      Serial.println(counts1);
      sensor1 = 2;
      logData = 1;
  }

  if (counts2 < countsAvg2-levelChange2)
  {  
      Serial.print("Sensor 2: Sitting, ");
      Serial.println(counts2);
      sensor2 = 1;
      logData = 1;
  }

   if (counts2 > countsAvg2 + levelChange2)
  {  
      Serial.print("Sensor 2: Got up, ");
      Serial.println(counts2);
      sensor2 = 2;
      logData = 1;
  }

  if (counts3 < countsAvg3-levelChange3)
  {  
      Serial.print("Sensor 3: Sitting, ");
      Serial.println(counts3);
      sensor3 = 1;
      logData = 1;
  }

   if (counts3 > countsAvg3 + levelChange3)
  {  
      Serial.print("Sensor 3: Got up, ");
      Serial.println(counts3);
      sensor3 = 2;
      logData = 1;
  }
  
  if (counts4 < countsAvg4-levelChange4)
  {  
      Serial.print("Sensor 4: Sitting, ");
      Serial.println(counts4);
      sensor4 = 1;
      logData = 1;
  }
  
   if (counts4 > countsAvg4 + levelChange4)
  {  
      Serial.print("Sensor 4: Got up, ");
      Serial.println(counts4);
      sensor4 = 2;
      logData = 1;
  }
  
  if (logData == 1)
  {  
    // fetch the time from RTC
    now = RTC.now();
    digitalWrite(redLEDpin, HIGH);

    // log time
    logfile.print(now.year(), DEC);
    logfile.print(",");
    logfile.print(now.month(), DEC);
    logfile.print(",");
    logfile.print(now.day(), DEC);
    logfile.print(",");
    logfile.print(now.hour(), DEC);
    logfile.print(",");
    logfile.print(now.minute(), DEC);
    logfile.print(",");
    logfile.print(now.second(), DEC);
    logfile.print(",");
    logfile.print(sensor1);
    logfile.print(",");
    logfile.print(sensor2);
    logfile.print(",");
    logfile.print(sensor3);
    logfile.print(",");
    logfile.print(sensor4);
    logfile.print(",");
    logfile.print(counts1);
    logfile.print(",");
    logfile.print(countsAvg1);
    logfile.print(",");
    logfile.print(counts2);
    logfile.print(",");
    logfile.print(countsAvg2);
    logfile.print(",");
    logfile.print(counts3);
    logfile.print(",");
    logfile.print(countsAvg3);
    logfile.print(",");
    logfile.print(counts4);
    logfile.print(",");
    logfile.print(countsAvg4);
    logfile.println();
 
     
    #if ECHO_TO_SERIAL
    Serial.print(now.year(), DEC);
    Serial.print(",");
    Serial.print(now.month(), DEC);
    Serial.print(",");
    Serial.print(now.day(), DEC);
    Serial.print(",");
    Serial.print(now.hour(), DEC);
    Serial.print(",");
    Serial.print(now.minute(), DEC);
    Serial.print(",");
    Serial.print(now.second(), DEC);
    Serial.print(",");
    Serial.print(sensor1);
    Serial.print(",");
    Serial.print(sensor2);
    Serial.print(",");
    Serial.print(sensor3);
    Serial.print(",");
    Serial.print(sensor4);
    Serial.print(",");
    Serial.print(counts1);
    Serial.print(",");
    Serial.print(countsAvg1);
    Serial.print(",");
    Serial.print(counts2);
    Serial.print(",");
    Serial.print(countsAvg2);
    Serial.print(",");
    Serial.print(counts3);
    Serial.print(",");
    Serial.print(countsAvg3);
    Serial.print(",");
    Serial.print(counts4);
    Serial.print(",");
    Serial.print(countsAvg4);
    Serial.println();
    #endif //ECHO_TO_SERIAL

   // For testing using the serial com port
   Serial.println("TRIGGERED!");
   Serial.println();

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  //if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  // blink LED to show we are syncing data to the card & updating FAT!
  logfile.flush();
  //digitalWrite(redLEDpin, LOW);
  logData = 0;
  delay(100);  //CHANGE ME if necessary
  }
 
}


