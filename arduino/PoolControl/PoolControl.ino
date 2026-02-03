//
// ValveAndTemp.ino
//
//   Control valves and monitor temperature.
//
#include "valve.h"
#include "thermometer.h"
#include "heater.h"
#include "pump.h"
#include "light.h"
#include "control.h"
#include <time.h>
#include "EEPROM.h"

Valve valve[] = {
  //    relay   relay   current  nv storage
  //   common direction monitor  eeprom address
  //   ------ --------- ------- ----------------
  Valve(   2,     3,       A7   ,  VALVE_EEPROM_ADDRESS),
  Valve(   4,     5,       A6   ,  VALVE_EEPROM_ADDRESS + VALVE_EEPROM_INCR)
};

Thermometer therm[] = {
  // thermometers are configured with the pin and the resistor
  //   that forms the voltage divider. The resistor is given in K ohms.
  Thermometer(A3,10,THERMOMETER_EEPROM_ADDRESS)
};

Heater heater[] = {
  // there is but one heater - so set it up with its relay pin
  //   and the termometer
  Heater(6,&therm[0],HEATER_EEPROM_ADDRESS)
};

Pump pump[] = {
  // we have two pumps: main and booster
  //   the main pump must be running for the booster to be running
  //   (note the two different signatures of constructors)
  Pump(   10,   11,   12, PUMP_EEPROM_ADDRESS),
  Pump(   A1, PUMP_EEPROM_ADDRESS + PUMP_EEPROM_INCR)
};

Light light[] = {
  Light(7, LIGHT_EEPROM_ADDRESS)
};

void setup()
{
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // set-up the control system through I2C. It needs to have access
  //   to the control "surface" so pass it arrays of valves and
  //   thermometers - note the counts of members in the array

  ControlSetup(valve,2,therm,1,heater,1,pump,2,light,1);
}

// the loop serves to process the ongoing state machines for
//   valves, as initiated by control.

void loop()
{
  valve[0].loop();
  valve[1].loop();
  heater[0].loop();
  therm[0].loop();
  pump[0].loop();
  pump[1].loop();
  light[0].loop();
}
