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
public:
  void factoryReset(void);


protected:
  int myAddress;	// my eeprom address that is used as the base for
                        //   storage for all parameters.

  EEPROM_CONTROL(int);

  int eepromHasBeenSet(void);

  int eepromWrite(int,int);
  int eepromWrite(int,byte);
  int eepromWrite(int,float);
  int eepromWrite(int,unsigned long);

  int eepromRead(int,int*);
  int eepromRead(int,byte*);
  int eepromRead(int,float*);
  int eepromRead(int,unsigned long *);
};


#endif
