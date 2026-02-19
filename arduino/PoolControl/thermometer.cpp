//
// thermometer.cpp
//
//   Implementation of the pool thermometer.  Taken from:
//   https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
//   Note, too, that calibration CAN be done by adjusting the
//   coeficients (below) - here is an npm libraray for it:
//   https://www.npmjs.com/package/steinhart-hart
//
//   Other standard coefficients for the Steinhart-Hart equation, for
//   a 10k resister, are found in:
//      https://www.qtisensing.com/wp-content/uploads/Steinhart-Hart-Coefficients.pdf
//
//   I measured the blue pool thermister and got:
//       34 deg = 27830 ohm
//	 68     = 12450
//      104     = 5540
//
//   For constants of 1.619370382e-3, 1.448585020e-4, 5.079933354e-7 (javascript)
//                    1.6137e-3       1.4584e-4       5.040e-7 (C++ small float)

#include <Arduino.h>
#include "steinharthart.h"

// this enum is used to index into the ThermCurves array

enum ThermCurveTypes {
    TC_S = 0,		// original curve from the tutorial above
    TC_S2 = 1,		// slightly different S curve from the pdf above
    TC_Z = 2,		// from pdf above
    TC_Y = 3,		// from pdf above
    TC_MEASURED = 4	// (see above)
};

struct
{	
    float c1;
    float c2;
    float c3;
} ThermCurves[] = {
    { 1.009249522e-03,    2.378405444e-04,    2.019202697e-07 },// S
    { 1.044054703604e-03, 2.34368328566e-04,  4.90829151e-07 },	// S2
    { 1.1164014655e-03,   2.37982973213e-04, -3.72283234e-07 },	// Z
    { 6.61913453349e-04,  3.26726406677e-04, -7.107372384e-06 },// Y
    { 1.619370382e-03,    1.448585020e-04,    5.079933354e-07 }	// MEASURED
};
    
#include <Arduino.h>
#include "thermometer.h"

//
// Thermometer() - simply configure the pin and the default
//     coefficients.
//
Thermometer::Thermometer(int pin, int kohms, int eepromAddress) : EEPROM_CONTROL(eepromAddress)
{
    int	curve = TC_MEASURED;
    myPin = pin;
    myResistor = (float)kohms * 1000.0;	// used during reading as a float -
                                        //   so go ahead and set it as such
  if(!eepromHasBeenSet()) {
      // set the defaults if none have been written to eeprom
      config(ThermCurves[curve].c1,ThermCurves[curve].c2,ThermCurves[curve].c3);

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

//
// coefficients() - given a byte buffer from an I2C read, translate
//    that information into coefficients, and then call config on
//    those.
//
//    Structure of the buffer:
//
//     Temp  Temp  Temp   Resist A    Resist B    Resist C
//      A(F)  B(F)  C(F)   hi   lo     hi   lo     hi   lo
//    |-----|-----|-----|-----,-----|-----,-----|-----,-----|
//
void Thermometer::coefficients(byte *buffer)
{
    byte tA = buffer[0];
    byte tB = buffer[1];
    byte tC = buffer[2];
    int rA = (((int)buffer[3]) << 8) | (int)buffer[4];
    int rB = (((int)buffer[5]) << 8) | (int)buffer[6];
    int rC = (((int)buffer[7]) << 8) | (int)buffer[8];
    float A, B, C;

    SHcoefficients(tA,tB,tC,rA,rB,rC,&A,&B,&C);

    Serial.println("Coefficients");
    Serial.println(A,10);
    Serial.println(B,10);
    Serial.println(C,10);
    
    config(A,B,C);
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
