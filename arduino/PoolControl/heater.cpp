//
// heater.cpp
//
//   Manage the heater, both making it active during "spa"
//   mode, and turning it on when the temp drops below the
//   set point.
//

#include "heater.h"
#include <Arduino.h>
#include <EEPROM.h>

// go ahead and adjust these two values if the relays change
#define RELAY_ON	LOW
#define RELAY_OFF	HIGH

// default set point - in tenths of degrees
#define DEFAULT_SETPOINT	700	// 70.0 degrees

// need some hysteresis to stop fast cycling of on/off (in tenths)
#define HOLD_OFF 20	// 2 degrees

Heater::Heater(int pin, Thermometer *therm, int eepromAddress) : EEPROM_CONTROL(eepromAddress)
{
  myPin = pin;
  myTherm = therm;
  myAddress = eepromAddress;

  if(!eepromHasBeenSet()) {
    config(DEFAULT_SETPOINT);		// set default if none is in eeprom
  } else {
    loadConfig();		// otherwise use eeprom value
  }
  
  pinMode(myPin,OUTPUT);

  enabled = 0;	// heater starts off disabled
  active = 0;	//   and inactive
}

//
// config() - configure the heating set point to the given
//    tenths of degrees. For example, 1001 means 100.1 degrees.
//
void Heater::config(int tenths)
{
  int offset = 0;
  
  setPoint = tenths;

  offset += eepromWrite(offset,setPoint);
}
void Heater::loadConfig()
{
  int offset = 0;

  offset += eepromRead(offset,&setPoint);
}

void Heater::enable(int onoff)
{
  enabled = onoff?1:0;
}

//
// loop() - this loop runs to turn on/off the heater based upon
//    the thermometer provided.
//
void Heater::loop(void)
{
  int reading;
  
  if(!enabled) {
    heatOFF();	// turn off relay if heater is disabled
    return;
  }

  // at this point we are enabled

  reading = myTherm->readAVG();

  // we build in the hysteresis here with the hold off
  if(active && reading >= (setPoint + HOLD_OFF)) {
    heatOFF();
  } else if(!active && reading < (setPoint - HOLD_OFF)) {
    heatON();
  }
}

//
// on() - turn on the heater - which means turn on the relay
//    and set active
//
void Heater::heatON(void)
{
  relayControl(RELAY_ON);
  active = 1;
}

//
// off() - turn off the heater
//
void Heater::heatOFF(void)
{
  relayControl(RELAY_OFF);
  active = 0;
}


//
// relayControl() - controls the given relay where:
//    relayPin - set to tiehter pinON or pinDIR for that relay
//    onoff - if TRUE turn on the relay, otherwise turn it off
//
void Heater::relayControl(int onoff)
{
  digitalWrite(myPin,onoff);
}
