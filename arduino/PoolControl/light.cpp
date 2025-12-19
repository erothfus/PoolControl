//
// light.cpp
//
//    Control the pool light(s?)! Simple relay.
//
#include "light.h"
#include <Arduino.h>

// go ahead and adjust these two values if the relays change
#define RELAY_ON	LOW
#define RELAY_OFF	HIGH

Light::Light(int pin, int eepromAddress)
{
  myPin = pin;
  myAddress = eepromAddress;

  relayControl(myPin,RELAY_OFF);
  pinMode(myPin,OUTPUT);

  status = 0;	// light starts off off
}

void Light::control(int onoff)
{
  relayControl(myPin,onoff?RELAY_ON:RELAY_OFF);
  status = onoff;
}

//
// loop() - nothing of note happens here
//
void Light::loop()
{
}

//
// relayControl() - controls the given relay where:
//
void Light::relayControl(int relayPin, int onoff)
{
  digitalWrite(relayPin,onoff);
}
  
