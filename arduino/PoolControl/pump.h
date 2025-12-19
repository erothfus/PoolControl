//
// pump.h
//
//   (see pump.cpp for more information)
//

#ifndef PUMP_H
#define PUMP_H

#define PUMP_MAIN	1
#define PUMP_BOOSTER	2

#define PUMP_SPEED_LOW	HIGH
#define PUMP_SPEED_HIGH	LOW

class Pump {

public:
  Pump(int,int,int,int);	// called for main pump
  Pump(int,int);		// called for booster pump

  void loop(void);

  void control(int);

  void factoryReset(void);

  int status;		// 0 = off, 1 = on-low-speed, 2 = on-hi-speed
     
private:
  int mode;		// either MAIN or BOOSTER

  int myAddress;		// eeprom storage address

  int blackRelay;		// 1/2 240v line relay (common for both pumps)
  int redRelay;		// other 1/2 240v line relay (diff per pump)
  int speedRelay;		// for main pump, this controls speed

  void relayControl(int,int);
	  
};


#endif
