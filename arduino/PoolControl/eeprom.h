//
// eeprom.h
//
//  (see eeprom.cpp)
//

#ifndef EEPROM_CONTROLH
#define EEPROM_CONTROLH

#include <Arduino.h>
#include <EEPROM.h>

class EEPROM_CONTROL {

protected:
  int myAddress;	// my eeprom address that is used as the base for
                        //   storage for all parameters.

  EEPROM_CONTROL(int);

  int eepromHasBeenSet(void);
  void factoryReset(void);

  int eepromWrite(int,int);
  int eepromWrite(int,byte);
  int eepromWrite(int,float);
  int eepromRead(int,int*);
  int eepromRead(int,byte*);
  int eepromRead(int,float*);
};


#endif
