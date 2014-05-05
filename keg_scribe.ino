#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <Time.h>
#include <SPI.h>
#include <Fat16.h>
#include <Fat16util.h> // use functions to print strings from flash memory

// for a smaller footprint
#define SERIAL_BUFFER_SIZE 32

/**********************************************************
This is example code for using the Adafruit liquid flow meters. 

Tested and works great with the Adafruit plastic and brass meters
    ------> http://www.adafruit.com/products/828
    ------> http://www.adafruit.com/products/833

Connect the red wire to +5V, 
the black wire to common ground 
and the yellow sensor wire to pin #2

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution
**********************************************************/

const char OK_MSG[]   = " ok.\r\n";
const char FAIL_MSG[] = " fail.\r\n";

// which pin to use for reading the sensor? can use any pin!
#define TEMPERATURE1_ANALOG_PIN    0
#define TEMPERATURE2_ANALOG_PIN    1
#define FLOWSENSOR_LED_DIGITAL_PIN 2
#define FLOWSENSOR1_DIGITAL_PIN    6
#define FLOWSENSOR2_DIGITAL_PIN    7

const char TEMPERATURE1_IMPORT_CODE[] = "KegScribeAmbientTemperature";
const char TEMPERATURE2_IMPORT_CODE[] = "KegScribeFridgeTemperature";
const char TAP1_IMPORT_CODE[]         = "KegScribeTap1";
const char TAP2_IMPORT_CODE[]         = "KegScribeTap2";

// number of milliseconds between recording to SD card
#define RECORD_INTERVAL (1000UL*60UL*1UL)
// number of milliseconds between reports
#define REPORT_INTERVAL (1000UL*60UL*5UL)
// number of seconds between calls to the NTP server
#define NTP_INTERVAL    (60*60*24)

void setup() {
  Serial.begin(115200);
  Serial.print(F("KegScribe 1.0\r\n")); 
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

// start at -1*INTERVAL so we always report/record at startup
unsigned long millisSinceLastReport = -1*REPORT_INTERVAL;
unsigned long millisSinceLastRecord = -1*RECORD_INTERVAL;

void loop() {
    
  // Get the current time
  time_t currentTime = now();
  
  // handle rollover of millis (after about 49 days)
  // by just reseting our last record time
  if (millis() < millisSinceLastRecord || millis() < millisSinceLastReport) {
    millisSinceLastRecord = 0;
    millisSinceLastReport = 0;
  }
  
  if (millis() > (millisSinceLastRecord + RECORD_INTERVAL)) {
    
    Serial.print(F("\r\n"));
  
    // Record Temperature
    recordValue(TEMPERATURE1_IMPORT_CODE, &currentTime, readTemperatureF(TEMPERATURE1_ANALOG_PIN));
    recordValue(TEMPERATURE2_IMPORT_CODE, &currentTime, readTemperatureF(TEMPERATURE2_ANALOG_PIN));
    
    // Report Tap 1 and reset the pulse count after a successful recording
    if(!recordValue(TAP1_IMPORT_CODE, &currentTime, readFlowSensor())) {
      resetFlowSensor();
    }
    
    millisSinceLastRecord = millis();
  }
  
  if (millis() > (millisSinceLastReport + REPORT_INTERVAL)) {
    // Report values to Hakase Server
    reportFiles();
    millisSinceLastReport = millis();
  }
  
}
