//
// EEPROM.h
//
//   A local include file that defines the locations that the other
//   modules can use to write their config data to EEPROM.
//
//   Each module currently gets 32 bytes to scribble into.
//
//   As a reference, for the Arduino nano the sizes of the types are
//   as follows. Note that there is no difference in size for unsigned
//   types.
//
//      type  bytes
//      ----  -----
//      byte  = 1 (duh)
//      char  = 1 (duh)
//	int   = 2
//	long  = 4
//	float = 4
//

#ifndef MYEEPROM_H
#define MYEEPROM_H

// the following defines set the EEPROM addresses for each of the
//   different modules. The _INCR is used to increment the address
//   when there are (or could be) multiple objects for the given
//   module.

#define VALVE_EEPROM_ADDRESS		0x10
#define VALVE_EEPROM_INCR		0x10

#define THERMOMETER_EEPROM_ADDRESS	0x30
#define TERMOMETER_EEPROM_INCR		0x10

#define HEATER_EEPROM_ADDRESS		0x50

#define PUMP_EEPROM_ADDRESS		0x70
#define PUMP_EEPROM_INCR		0x10

#define LIGHT_EEPROM_ADDRESS		0x80
#define LIGHT_EEPROM_INCR		0x10

#endif
