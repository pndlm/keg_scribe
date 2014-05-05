

float readTemperatureF(int sensorPin) {
  int reading = analogRead(sensorPin);  
  
  // see explanation below
  return (((((analogRead(sensorPin) * 5.0)/1024.0) - 0.5) * 100) * 9.0 / 5.0) + 32.0;
  
  /*
  // converting that reading to voltage, for 3.3v arduino use 3.3
  float voltage = (reading * 5.0);
  voltage /= 1024.0; 
  
  // now print out the temperature
  float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
  
  // now convert to Fahrenheit
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  
  return temperatureF;
  */
}
