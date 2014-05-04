#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <Time.h>
#include <SPI.h>
#include <Fat16.h>
#include <Fat16util.h> // use functions to print strings from flash memory

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
//#include "LiquidCrystal.h"
//LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define OK_MSG     (" ok.\r\n")
#define FAIL_MSG   (" fail.\r\n")

// which pin to use for reading the sensor? can use any pin!
#define TEMPERATURE1_ANALOG_PIN 0
#define TEMPERATURE2_ANALOG_PIN 1
#define FLOWSENSOR_DIGITAL_PIN 2

#define TEMPERATURE1_IMPORT_CODE "KegScribeAmbientTemperature"
#define TEMPERATURE2_IMPORT_CODE "KegScribeFridgeTemperature"
#define TAP1_IMPORT_CODE "KegScribeTap1"
#define TAP2_IMPORT_CODE "KegScribeTap2"

// number of milliseconds between recording values
#define LOOP_INTERVAL (1000)
// number of milliseconds between recording to SD card
#define RECORD_INTERVAL (1000UL*10UL*1UL)
// number of milliseconds between reports
#define REPORT_INTERVAL (1000UL*60UL*5UL)
// number of seconds between calls to the NTP server
#define NTP_INTERVAL (60*60*24)

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSOR_DIGITAL_PIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void setup() {
  Serial.begin(115200);
  
  Serial.print(F("KegScribe 1.0\r\n")); 
  //Serial.print(F("Free RAM ")); Serial.print(getFreeRam(), DEC);
  
  //char buffer[200];
  //cbPrintULong(buffer, 1234UL);
  //Serial.print(buffer);
  
  initWifi();
  initSD();
  
  pinMode(FLOWSENSOR_DIGITAL_PIN, INPUT);
  digitalWrite(FLOWSENSOR_DIGITAL_PIN, HIGH);
  lastflowpinstate = 0; digitalRead(FLOWSENSOR_DIGITAL_PIN);
  useInterrupt(true);
  
  setSyncProvider(getNtpTime);
  setSyncInterval(NTP_INTERVAL);
}

// start at -1*REPORT_INTERVAL so we always report at startup
unsigned long millisSinceLastReport = -1*REPORT_INTERVAL;

void loop() {
  
  if (timeStatus() == timeNotSet) {
    Serial.print("time not set.\r\n");
  }
    
  float temperature1 = readTemperatureF(TEMPERATURE1_ANALOG_PIN);
  float temperature2 = readTemperatureF(TEMPERATURE2_ANALOG_PIN);
  float tap1L = readTapLiters();
  
  time_t currentTime = now();
  
  char timestamp[20];
  sprintTime(timestamp, &currentTime, false);
  Serial.print("timestamp ");Serial.print(timestamp); Serial.print(F("\r\n"));
  
  Serial.print(temperature1); Serial.print(F("°F (Ambient)\r\n"));
  Serial.print(temperature2); Serial.print(F("°F (Fridge)\r\n"));
  Serial.print(tap1L); Serial.print(F("l\r\n"));
  
  // Record Temperature
  recordValue(TEMPERATURE1_IMPORT_CODE, &currentTime, temperature1);
  recordValue(TEMPERATURE2_IMPORT_CODE, &currentTime, temperature2);
  
  // Report Tap 1
  if(!recordValue(TAP1_IMPORT_CODE, &currentTime, tap1L)) {
    // reset the pulse counts after a successful report
    pulses = 0;
  }
  
  if (millis() > (millisSinceLastReport + REPORT_INTERVAL)) {
    // Report values to Hakase Server
  
    Serial.print(tap1L); Serial.print("Sending data...\r\n");
    reportFiles();
    millisSinceLastReport = millis();
  }
 
  delay(RECORD_INTERVAL);
}

float readTapLiters() {
  //lcd.setCursor(0, 0);
  //lcd.print("Pulses:"); 
  //lcd.print(pulses, DEC);
  //lcd.print(" Hz:");
  //lcd.print(flowrate);
  //lcd.print(flowrate);
  //Serial.print("Freq: "); 
  //Serial.println(flowrate);
  //Serial.print("Pulses: "); 
  //Serial.println(pulses, DEC);
  
  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

/*
  // if a brass sensor use the following calculation
  float liters = pulses;
  liters /= 8.1;
  liters -= 6;
  liters /= 60.0;
*/
  //lcd.setCursor(0, 1);
  //lcd.print(liters); lcd.print(" Liters        ");

  return liters;  
}

float readTemperatureF(int sensorPin) {
  int reading = analogRead(sensorPin);  
  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  
  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
  
  // now convert to Fahrenheit
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  
  return temperatureF;
}
