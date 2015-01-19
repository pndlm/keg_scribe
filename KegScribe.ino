#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <Time.h>
#include <SPI.h>
#include <Fat16.h>
#include <Fat16util.h>

// for a smaller footprint
#define SERIAL_BUFFER_SIZE 32

// success and failure messages
// and other reoccuring strings
// used throughout the application
const char OK_MSG[]   = " ok\r\n";
const char FAIL_MSG[] = " fail\r\n";
const char NEWLINE_MSG[] = "\r\n";
const char CSV_SEPARATOR = ',';

// change these pin numbers to fit your needs
// since we use the CC3000 Wifi Shield these
// are the ideal pin settings for us
#define TEMPERATURE1_ANALOG_PIN    0
#define TEMPERATURE2_ANALOG_PIN    1
#define FLOWSENSOR_LED_DIGITAL_PIN 2
#define FLOWSENSOR1_DIGITAL_PIN    6
#define FLOWSENSOR2_DIGITAL_PIN    7

// These are strings that uniquely identify
// the different data feeds to our web server
#define TEMPERATURE1_IMPORT_CODE "KegScribeAmbientTemperature"
#define TEMPERATURE2_IMPORT_CODE "KegScribeFridgeTemperature"
#define TAP1_IMPORT_CODE         "KegScribeTap1"
#define TAP2_IMPORT_CODE         "KegScribeTap2"

// number of milliseconds between recording to SD card
#define RECORD_INTERVAL (1000UL*60UL*5UL)

// number of milliseconds between reports to the web server
#define REPORT_INTERVAL (1000UL*60UL*1UL)

// number of seconds between calls to the NTP server
#define NTP_INTERVAL    (60*60*24)

// These arrays are used to define the
// various sensor pins and import codes.
// Do not change these arrays unless you
// need to add additional sensors. Instead
// modify the defines above.

const byte tempSensorPin[2] = {
  TEMPERATURE1_ANALOG_PIN,
  TEMPERATURE2_ANALOG_PIN
};

const byte flowSensorPin[2] = {
  FLOWSENSOR1_DIGITAL_PIN,
  FLOWSENSOR2_DIGITAL_PIN
};

const char* tempSensorCode[2] = {
  TEMPERATURE1_IMPORT_CODE,
  TEMPERATURE2_IMPORT_CODE
};

const char* flowSensorCode[2] = {
  TAP1_IMPORT_CODE,
  TAP2_IMPORT_CODE
};

void setup() {
  Serial.begin(115200);
  Serial.print(F("KegScribe 1.1\r\n")); 
  
  //Serial.print(F("FreeRam: "));
  //Serial.print(FreeRam(), DEC);
  //Serial.print(NEWLINE_MSG);
  
  // Fat16 lib may need
  // to be inited first.
  initSD();
  
  for(byte i = 0; i < sizeof(tempSensorPin); i++) {
    initFlowSensor(i);
  }
  
  initWifi();
  initTime();
}

unsigned long millisSinceLastReport = 0;

void loop() {
    
  // Get the current time
  time_t currentTime = now();
  
  Serial.print(NEWLINE_MSG);

  for(byte i = 0; i < sizeof(flowSensorPin); i++) {
     recordValue(tempSensorCode[i], &currentTime, readTemperatureF(tempSensorPin[i]));
  }
  
  for(byte i = 0; i < sizeof(tempSensorPin); i++) {
    // Report flow sensor values and reset the pulse count after a successful recording
    if(!recordValue(flowSensorCode[i], &currentTime, readFlowSensor(i))) {
      resetFlowSensor(0);
    }
  }
  
  // handle rollover of millis (after about 49 days)
  // by just reseting our last report time
  if (millis() < millisSinceLastReport) {
    millisSinceLastReport = 0;
  }
  
  if (millis() > (millisSinceLastReport + REPORT_INTERVAL)) {
    // Report values to Hakase Server
    reportFiles();
    millisSinceLastReport = millis();
  }
  
  delay(RECORD_INTERVAL);
  
}
