//
// heater.h
//
//   (see heater.cpp for more information)
//

#include "thermometer.h"
#include "eeprom.h"

#ifndef HEATER_H
#define HEATER_H

class Heater : public EEPROM_CONTROL {

public:
  int enabled;	// heater is enabled
  int active;	// heater is currently active - "on" / "heating"

  int setPoint;	// heating set point in tenths of degrees - 105 degrees is 1050

  void loop(void);	// heat management loop

  Heater(int,Thermometer *,int);
  void config(int);
  void enable(int);

private:
  int	       myPin;		// pin to turn on the heat
  Thermometer *myTherm;		// the thermometer to use for heat control

  void relayControl(int);   	// internal relay control
  void heatON(void);
  void heatOFF(void);
  void loadConfig(void);
};


#endif
