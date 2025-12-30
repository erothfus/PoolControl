//
// thermometer.h
//
//   (see temp.cpp for information about this class)
//

#ifndef THERMOMETER_H
#define THERMOMETER_H

#define READINGS_FOR_AVERAGE	20

#include <Arduino.h>
#include "eeprom.h"

class Thermometer : protected EEPROM_CONTROL {

public:

  Thermometer(int,int,int);
  void readI2C(byte *);	// this read is for I2C return - uses readAVG()
  int read(void);	// return tenths of degrees (1000 => 100.0)
  int readAVG(void);	// returns the average of the last READINGS_FOR_AVERAGE readings (use this)
  void loop(void);	// used to keep the average up

  // configuration for a thermometer means giving it the three
  //   constants used for termister temp translation
  
  void config(float,float,float);
  void loadConfig(void);
  
private:
  int myPin;
  float myResistor;	// this is kept as a float because that's how it is used
  int myAddress;	// EEPROM address for NV storage for this object config

  // the following constants are used to convert the analog
  //   reading to a temp
     
  float c1,c2,c3;

  // we need to average the temp readings so that the outlyers don't cause
  //   heating to fast cycle
  int readings[READINGS_FOR_AVERAGE];
  int readingPointer;
  long readingTotal;
  int readingAverage;

};

#endif

