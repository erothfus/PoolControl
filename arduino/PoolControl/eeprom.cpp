//
// eeprom.cpp
//
//   Implements the basic eeprom functions that all resources use.
//
#include "eeprom.h"

EEPROM_CONTROL::EEPROM_CONTROL(int addr)
{
  myAddress = addr;
}

void EEPROM_CONTROL::factoryReset()
{
  EEPROM.write(myAddress,(byte)255);
}

//
// eepromHasBeenSet() - returns true/false depending upon whether the eeprom
//    as been written for this object (if the first byte is 255). Note that this
//    is how factoryReset is accomplished - that byte is changed to 255.
//
int EEPROM_CONTROL::eepromHasBeenSet()
{
  return(EEPROM.read(myAddress) != (byte)255);
}

int EEPROM_CONTROL::eepromWrite(int offset,byte value)
{
  EEPROM.put(myAddress,(byte)1);
  EEPROM.put(myAddress+1+offset,value);
  return(sizeof(value));
}

int EEPROM_CONTROL::eepromWrite(int offset,int value)
{
  EEPROM.put(myAddress,(byte)1);
  EEPROM.put(myAddress+1+offset,value);
  return(sizeof(value));
}

int EEPROM_CONTROL::eepromWrite(int offset,float value)
{
  EEPROM.put(myAddress,(byte)1);
  EEPROM.put(myAddress+1+offset,value);
  return(sizeof(value));
}

int EEPROM_CONTROL::eepromWrite(int offset,unsigned long value)
{
  EEPROM.put(myAddress,(byte)1);
  EEPROM.put(myAddress+1+offset,value);
  return(sizeof(value));
}

int EEPROM_CONTROL::eepromRead(int offset, byte *retValue)
{
  EEPROM.get(myAddress+1+offset,*retValue);
  return(sizeof(*retValue));
}
int EEPROM_CONTROL::eepromRead(int offset, int *retValue)
{
  EEPROM.get(myAddress+1+offset,*retValue);
  return(sizeof(*retValue));
}
int EEPROM_CONTROL::eepromRead(int offset, float *retValue)
{
  EEPROM.get(myAddress+1+offset,*retValue);
  return(sizeof(*retValue));
}
int EEPROM_CONTROL::eepromRead(int offset, unsigned long *retValue)
{
  EEPROM.get(myAddress+1+offset,*retValue);
  return(sizeof(*retValue));
}
