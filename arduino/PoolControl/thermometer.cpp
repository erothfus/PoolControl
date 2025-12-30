//
// thermometer.cpp
//
//   Implementation of the pool thermometer.  Taken from:
//   https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
//   Note, too, that calibration CAN be done by adjusting the
//   coeficients (below) - here is an npm libraray for it:
//   https://www.npmjs.com/package/steinhart-hart
//
#include <Arduino.h>
#include "thermometer.h"

//
// Thermometer() - simply configure the pin and the default
//    factors.
//
Thermometer::Thermometer(int pin, int kohms, int eepromAddress) : EEPROM_CONTROL(eepromAddress)
{
  myPin = pin;
  myResistor = (float)kohms * 1000.0;	// used during reading as a float -
                                        //   so go ahead and set it as such

  if(!eepromHasBeenSet()) {
    // set the defaults if none have been written to eeprom
    config( 1.009249522e-03, 2.378405444e-04, 2.019202697e-07);
  } else {
    // otherwise load them from eeprom
    loadConfig();
  }

  // set up for reading averaging
  readingPointer = -1;
}

//
// loadConfig() - load the default constants from EEPROM. This will happen
//   upon the first arduino load, or after a "factorReset".
//
void Thermometer::loadConfig()
{
  int offset = 0;

  offset += eepromRead(offset,&c1);
  offset += eepromRead(offset,&c2);
  offset += eepromRead(offset,&c3);
}  
void Thermometer::config(float con1, float con2, float con3)
{
  int offset = 0;
  
  c1 = con1;
  c2 = con2;
  c3 = con3;

  offset += eepromWrite(offset,c1);
  offset += eepromWrite(offset,c2);
  offset += eepromWrite(offset,c3);
}

//
// readI2C() - read this thermometer, returning the read temp
//    in a byte buffer for transmission over I2C.
//
void Thermometer::readI2C(byte *buffer)
{
  buffer[0] = (byte)((readingAverage >> 8)&0xff);
  buffer[1] = (byte)(readingAverage &0xff);
}

//
// read() - read the given thermometer, returning a temp in
//    tenths of degrees (805 = 80.5 F).
//
int Thermometer::read(void)
{
  float	reading;
  float logR2, R2, T;
  
  // get the reading, which is between 0 and 1023 inclusive
  //   which represents the voltage at the voltage divider
  //   created with myResistor
 
  reading = (float) analogRead(myPin);
  
  R2 = myResistor * (1023.0 / reading - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));

  T = T - 273.15;		// Kelvins to Celsius
  T = (T * 9.0)/ 5.0 + 32.0;	// Celsius to Fahrenheit

  // now, throw out all of that float stuff and return an
  //   int - since our temp is (usefully) between -20 and
  //   110 degrees (F) we can easily return a temp in
  //   tenths of a degree in a 2-byte signed int

  return((int)(T * 10.0));
}

int Thermometer::readAVG()
{
  return(readingAverage);
}

//
// loop() - the purpose of the Thermometer loop is to read the thermometer
//    and average the readings. It appears that they can jump about a bit.
//    So we set the average reading count, and keep the local average up
//    to date with the loop. Callers can ask for a reading or for the
//    average.
//
void Thermometer::loop()
{
  int i;
  int reading;
  int oldReading;

  reading = read();	// get reading in tenths of degree
  //  Serial.print(reading);
  //  if(reading > 700) {
  //    Serial.print(" **** ");
  //  } else {
  //    Serial.print(" ");
  //  }

  if(readingPointer == -1) {
    // first reading, just fill the array
    for(i=0; i < READINGS_FOR_AVERAGE; i++) {
      readings[i] = reading;
    }
    readingPointer = 1;
    readingAverage = reading;
    readingTotal = READINGS_FOR_AVERAGE * reading;
  } else {
    oldReading = readings[readingPointer];
    readings[readingPointer] = reading;
    readingTotal = readingTotal - oldReading + reading;
    readingAverage = readingTotal / READINGS_FOR_AVERAGE;
    readingPointer = (readingPointer+1)%READINGS_FOR_AVERAGE;
  }

  // Serial.println(readingAverage);
}
