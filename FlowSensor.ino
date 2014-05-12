/*
Functions to read data from the flow sensor

This flow sensor code we used is for the plastic flow meter
available from Adafruit. The license and redemo details are
included below but the code has been changed slightly to
fit our needs.
*/

/**********************************************************
This is example code for using the Adafruit liquid flow meters. 

Tested and works great with the Adafruit plastic and brass meters
    ------> http://www.adafruit.com/products/828
    ------> http://www.adafruit.com/products/833

Connect the red wire to +5V, the black wire to common ground and the yellow sensor wire to pin #2

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution

license.txt
-----------

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************/

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
//volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
//volatile float flowrate;

void initFlowSensor() {
  // LED off
  pinMode(FLOWSENSOR_LED_DIGITAL_PIN, OUTPUT);
  digitalWrite(FLOWSENSOR_LED_DIGITAL_PIN, LOW);
  
  pinMode(FLOWSENSOR1_DIGITAL_PIN, INPUT);
  digitalWrite(FLOWSENSOR1_DIGITAL_PIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSOR1_DIGITAL_PIN);
  useInterrupt(true);
}

// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSOR1_DIGITAL_PIN);
  
  if (x == lastflowpinstate) {
    //lastflowratetimer++;
    digitalWrite(FLOWSENSOR_LED_DIGITAL_PIN, LOW);
    return; // nothing changed!
  }
  
  digitalWrite(FLOWSENSOR_LED_DIGITAL_PIN, HIGH);
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
    
  lastflowpinstate = x;
  //flowrate = 1000.0;
  //flowrate /= lastflowratetimer;  // in hertz
  
  //lastflowratetimer = 0;
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

/* returns the amount of flowed liquid in liters */
float readFlowSensor() {
  // if a plastic sensor use the following calculation
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  return ((pulses/7.5)/60.0);

  /*
    // if a brass sensor use the following calculation
    float liters = pulses;
    liters /= 8.1;
    liters -= 6;
    liters /= 60.0;
  */
}

/* Reset the flow sensor count */
void resetFlowSensor() {
  pulses = 0;
}
