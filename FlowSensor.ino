
// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;

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
    lastflowratetimer++;
    digitalWrite(FLOWSENSOR_LED_DIGITAL_PIN, LOW);
    return; // nothing changed!
  }
  
  digitalWrite(FLOWSENSOR_LED_DIGITAL_PIN, HIGH);
  
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
