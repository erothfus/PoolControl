//
// pump.cpp
//
//   Controller for the pumps.
//
//   Pumps are pretty boring. The main pump has two speeds,
//   and the booster pump has just one.
//
#include "pump.h"
#include <Arduino.h>

// go ahead and adjust these two values if the relays change
#define RELAY_ON	LOW
#define RELAY_OFF	HIGH

//
// Pump() - there are two pumps supported, a 240v 2-speed and a
//    24v single speed.
//
//    Specifically for this implementation, there is a main pump
//    and a booster (Polaris) pump. The main pump MUST be on before
//    the booster pump will work. This is effected by having both
//    pumps use the same pinOnBlack relay, but only the main pump
//    controlling it.
//
//    So there are two different constructors: one for the main
//    and one for the booster. The right mode gets set for the
//    specific pump in their constructors.
//

// MAIN PUMP
Pump::Pump(int pinOnBlack, int pinOnRed, int pinSpeed, int eepromAddress)
{
  mode = PUMP_MAIN;
  myAddress = eepromAddress;	// no configuration for pumps
  blackRelay = pinOnBlack;
  redRelay = pinOnRed;
  speedRelay = pinSpeed;

  relayControl(blackRelay,RELAY_OFF);
  relayControl(redRelay,RELAY_OFF);
  relayControl(speedRelay,RELAY_OFF);
  
  pinMode(blackRelay,OUTPUT);
  pinMode(redRelay,OUTPUT);
  pinMode(speedRelay,OUTPUT);

}

//
// factoryReset() - no configuration information needed, so nothing
//    happens for the factory reset.
//
void Pump::factoryReset(void)
{
}

// BOOSTER PUMP
Pump::Pump(int pinOnRed, int eepromAddress)
{
  mode = PUMP_BOOSTER;
  myAddress = eepromAddress;
  redRelay = pinOnRed;
  status = 0;

  Serial.print("pump init, status "); Serial.println(status);
  relayControl(redRelay,RELAY_OFF);
  
  pinMode(redRelay,OUTPUT);
}

//
// control() - control this pump. Takes a speed. 0 (zero) is off,
//    1 is low speed, and 2 is high speed.
//    NOTE that the PUMP_BOOSTER mode will NOT turn on the black relay. It
//    needs to be already one - ie the PUMP_MAIN must be on.
//
void Pump::control(int speed)
{
  Serial.print("pump speed ");
  Serial.println(speed);
  
  status = speed;

  switch(mode) {
  case PUMP_MAIN:
    relayControl(blackRelay,(speed != 0)?RELAY_ON:RELAY_OFF);
    relayControl(redRelay,(speed !=0)?RELAY_ON:RELAY_OFF);
    relayControl(speedRelay,(speed != 0 && speed == 2)?RELAY_ON:RELAY_OFF);
    break;
  case PUMP_BOOSTER:
    relayControl(redRelay,(speed != 0)?RELAY_ON:RELAY_OFF);
    break;
  }

}

//
// loop() - nothing happens in the pump loop
//
void Pump::loop()
{
}


//
// relayControl() - controls the given relay where:
//
void Pump::relayControl(int relayPin, int onoff)
{
  digitalWrite(relayPin,onoff);
}
