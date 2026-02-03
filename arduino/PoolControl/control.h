//
// control.h
//
#include "valve.h"
#include "thermometer.h"
#include "pump.h"
#include "heater.h"
#include "light.h"

extern void ControlSetup(Valve *, int ,
			 Thermometer *, int,
			 Heater *, int,
			 Pump *, int,
			 Light *, int);

extern void FactoryReset(void);

#define SLAVE_ADDR	0x20
