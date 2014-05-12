#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <Time.h>
#include <SPI.h>
#include <Fat16.h>
#include <Fat16util.h>

// for a smaller footprint
#define SERIAL_BUFFER_SIZE 32

// success and failure messages
// used throughout the application
const char OK_MSG[]   = " ok.\r\n";
const char FAIL_MSG[] = " fail.\r\n";

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
const char TEMPERATURE1_IMPORT_CODE[] = "KegScribeAmbientTemperature";
const char TEMPERATURE2_IMPORT_CODE[] = "KegScribeFridgeTemperature";
const char TAP1_IMPORT_CODE[]         = "KegScribeTap1";
const char TAP2_IMPORT_CODE[]         = "KegScribeTap2";

// number of milliseconds between recording to SD card
#define RECORD_INTERVAL (1000UL*60UL*1UL)

// number of milliseconds between reports to the web server
#define REPORT_INTERVAL (1000UL*60UL*5UL)

// number of seconds between calls to the NTP server
#define NTP_INTERVAL    (60*60*24)

void setup() {
  Serial.begin(115200);
  Serial.print(F("KegScribe 1.1\r\n")); 
  Serial.print(F("FreeRam: "));
  Serial.print(FreeRam(), DEC);
  Serial.print(F("\r\n"));
  
  // Fat16 lib may need
  // to be inited first.
  initSD();
  initFlowSensor();
  initWifi();
  initTime();
}

unsigned long millisSinceLastReport = 0;

void loop() {
    
  // Get the current time
  time_t currentTime = now();
  
  Serial.print(F("\r\n"));

  // Record Temperature
  recordValue(TEMPERATURE1_IMPORT_CODE, &currentTime, readTemperatureF(TEMPERATURE1_ANALOG_PIN));
  recordValue(TEMPERATURE2_IMPORT_CODE, &currentTime, readTemperatureF(TEMPERATURE2_ANALOG_PIN));
  
  // Report Tap 1 and reset the pulse count after a successful recording
  if(!recordValue(TAP1_IMPORT_CODE, &currentTime, readFlowSensor())) {
    resetFlowSensor();
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
