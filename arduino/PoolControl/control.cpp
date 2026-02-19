//
// control.cpp
//
//   Implements the external control of the functions and data
//   provided by the arduino, including things such as moving valves
//   and checking temperature.
//
//   Control is implemented as I2C callbacks. The callbacks are
//   processed as ISRs so they don't do much on their own for the big
//   control mechanisms. For simple reads, though, they happen inside
//   the ISR.
//
//   Control is implemented as a set of global routines, particularly
//   the ISR callbacks.
//
//   Control is organized kinda' like reading and writing registers.
//   To make something happen, a write occurs to a given register.
//   To read something, a write occurs, of the "register" number,
//   followed sending data back.
//
//   The "registers" are:
//
//     command w/r  arg target
//       ...    .    .    .
//      /   \   |   / \  / \
//      7 6 5   4   3 2  1 0
//      -----   -   ---  ---
// x    0 0 0   0   0 0  x y  - read status of valve [target] (4 bytes returned)
//      0 0 0   0   0 1  x y  - read [target] travel times (4 bytes returned)
//      0 0 0   1/0 1 0  x y  - write/read [target] min degrees (1)
//      0 0 0   1/0 1 1  x y  - write/read [target] max degrees (1)
// x    0 0 1   1   0 0  x y  - initiate calibration on valve [target] (0)
//      0 1 0   1   0 0  x y  - move [target] to given degrees (1)
//      0 1 0   1   0 1  x y  - move [target] to min degrees (0)
//      0 1 0   1   1 0  x y  - move [target] to max degrees (0)
//
//      0 1 1   1   0 0  x y  - set speed zero (off) on [target]
//      0 1 1   1   0 1  x y  - set speed one (slow) on [target]
//      0 1 1   1   1 0  x y  - set speed two (fast) on [target]
//      0 1 1   0   X X  x y  - get speed status of pump [target]
//
//      1 0 0   0   0 0  x y  - read temperature of [target] (2)
//      1 0 0   1   X X  x y  - configure therm coefficients (9 bytes)
//
//	1 0 1   1   0 0  x y  - heater off of [target]
//	1 0 1   1   0 1  x y  - heater on of [target]
//      1 0 1   1   1 0  x y  - config set point of heater [target]
//      1 0 1   0   X X  x y  - heater status (on or off) of [target]
//
//      1 1 0   1   0 0  x y  - light off for [target]
//      1 1 0   1   0 1  x y  - light on for [target]
//      1 1 0   0   0 0  x y  - status of light [target]
//
//      1 1 1   1   X X  X X  - eeprom factory reset
//      
#include "control.h"
#include <Arduino.h>      // can go away later
#include <Wire.h>

byte targetRegister;	// set to the first byte of any write - necessary to
                        //   identify the register that is being read

Valve *valves;
int valveCount;

Thermometer *therms;
int thermCount;

Heater *heaters;
int heaterCount;

Pump *pumps;
int pumpCount;

Light *lights;
int lightCount;

//
// ControlRegisterWrite() - an incoming write was received. This means
//    one of two different things:
//
//    - data is being WRITTEN to a "register" - the first byte is the
//      register, and further bytes are the data.
//
//    - data is being READ from a "register" - this write is a one-byte
//      register number. This will be followed by a read. So the
//      register number is kept in a global.
//
void ControlRegisterWrite(int count)
{
  byte incoming;
  int command;
  int isWrite;
  int arg;
  int i;
  int target;
  int degrees;	// used to assemble the move-to degrees
  byte dataBuffer[10];		// simple data buffer - should be using this  
  
  //  Serial.print("Write Received:"); Serial.println(count);

  if(count > 0) {
    targetRegister = Wire.read();
    count--;

    command = (targetRegister >> 5) & 0x07;
    isWrite = (targetRegister >> 4) & 0x01;
    arg = (targetRegister >> 2) & 0x03;
    target = targetRegister & 0x03;

    if(isWrite) {
    
      // the following switch/table implements the registers that can be
      //   written - if the incoming targetRegister IS a write.  If the
      //   incoming request is actually a read, this write will store
      //   the target register and a request for data will come in
      //   right afterwards, and that routine will use "targetRegister"
      //   to decide what to send back.
    
      switch(command) {

	// write valve config param
      case 0b000:
	switch(arg) {
	case 0x00:  Serial.print("ERROR (can't write status) "); break;   // one more byte
	case 0x01:  Serial.print("direction "); break;   // one more byte
	case 0x02:  Serial.print("min degrees "); break; // one more byte
	case 0x03:  Serial.print("max degrees "); break; // one more byte
	}
	break;

	// initiate calibration	
      case 0b001:
	valves[target].calibrate();
	break;

	// initiate valve move	
      case 0b010:
	if(count > 1) {
	  degrees = Wire.read() << 8;
	  degrees |= Wire.read();
	  count -= 2;
	  valves[target].move(degrees);
	  
	  //	  switch(arg) {
	  //	  case 0x00:  Serial.print("to degrees "); break;   // one more byte
	  //	  case 0x01:  Serial.print("to min degrees "); break;
	  //	  case 0x02:  Serial.print("to max degrees "); break;
	  //      }
	}
	break;

	// set pump speed - 0 off, 1 low, 2 high
      case 0b011:
	pumps[target].control(arg);
	break;

	// configure thermometer coefficients
      case 0b100:
	  Serial.println("therm coef config");
	  if(count >= 9) {
	      for(i=0; i < 9; i++) {
		  dataBuffer[i] = Wire.read();
	      }
	      count -= 9;
	      therms[target].coefficients(dataBuffer);
	  }
	  break;
	  
	// work with the heater
      case 0b101:
	switch(arg) {

	case 0b00:
	case 0b01:
	  heaters[target].enable(arg);
	  break;

	  // configure heater set temp - 2 bytes of tenths of degrees
	case 0b10:
	  Serial.println("heater config");
	  if(count > 1) {
	    degrees = Wire.read() << 8;
	    degrees |= Wire.read();
	    count -= 2;
	    Serial.println(degrees);
	    heaters[target].config(degrees);
	  }
	  break;
	}
	break;

	// control the light - arg is on or off (1 or 0)
      case 0b110:
	lights[target].control(arg);
	break;

	// factory reset of eeprom
      case 0b111:
	Serial.println("factory reset");
	FactoryReset();
	break;
      }

    }
    while(count--) {
      Wire.read();	// dump any extra data
    }
  }
}

//
// ControlRegisterRead() - this is a request to read a particular
//   "register". The register identifier was given in the previous
//   write, and is now in "targetRegister".
//
void ControlRegisterRead()
{
  int command;
  int isWrite;
  int arg;
  int target;
  byte valveStatus[4];		// used to report current status (cur,prev,deg)
  byte travelTimes[4];		// used to report current calibration travel times
  byte degrees[] = {0x01,0x02};	// used to report current degrees
  byte tempReading[2];
  byte singleByteData[1];
  byte dataBuffer[10];		// simple data buffer - should be using this
  
  //  Serial.print("Read Received ");  Serial.println(targetRegister);

  command = (targetRegister >> 5) & 0x07;
  isWrite = (targetRegister >> 4) & 0x01;
  arg = (targetRegister >> 2) & 0x03;
  target = targetRegister & 0x03;

  if(!isWrite) {

    switch(command) {

      // read valve config param
    case 0x00:
      switch(arg) {
      case 0x00:  // send four bytes (cur,prev,deg)
	valves[target].status(valveStatus);
	Wire.write(valveStatus,4);
	break;
	  
      case 0x01:
	valves[target].travelTime(travelTimes);
	Wire.write(travelTimes,4);
	break;

      case 0x02:
      case 0x03:
	valves[target].degreesGet(arg&0x01,degrees); // MIN (for 0x02) or MAX (0x03)
	Wire.write(degrees,2);
	break;
      }
      break;

    // get pump speed
    case 0b011:
	singleByteData[0] = pumps[target].status;
	Wire.write(singleByteData,1);
	break;
      
      // read temperature
    case 0b100:
      therms[target].readI2C(tempReading);
      Wire.write(tempReading,2);
      break;

    // read heater status
    case 0b101:
      dataBuffer[0] = heaters[target].enabled;
      dataBuffer[1] = heaters[target].active;
      dataBuffer[2] = heaters[target].setPoint >> 8;
      dataBuffer[3] = heaters[target].setPoint & 0xff;
      Wire.write(dataBuffer,4);
      break;

    // read light status
    case 0b110:
      dataBuffer[0] = lights[target].status;
      Wire.write(dataBuffer,1);
      break;

    }
  }

}


  

//
// ControlSetup() - set-up control using the wire library and I2C.
//     Control gets pointers to all of the "objects" so that it can
//     dispatch messages, both ways, based upon querying those
//     objects.
//
void ControlSetup(Valve *valve, int vCount ,
		  Thermometer *therm, int tCount,
		  Heater *htr, int hCount,
		  Pump *pump, int pCount,
		  Light *light, int lCount
		  )
{
  valves = valve;
  valveCount = vCount;

  therms = therm;
  thermCount = tCount;

  heaters = htr;
  heaterCount = hCount;

  pumps = pump;
  pumpCount = pCount;

  lights = light;
  lightCount = lCount;
  
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(ControlRegisterWrite);
  Wire.onRequest(ControlRegisterRead);

  Serial.println("control up dude");
}

void (*ResetFunction)(void) = 0;	// simple function to software reboot

void FactoryReset()
{
  int i;

  // only call factory reset for those using EEPROM
  
  for(i=0; i < valveCount; i++) {	// travel limits, position, travel times
    valves[i].factoryReset();
  }

  for(i=0; i < thermCount; i++) {	// coefficients for thermocouple
    therms[i].factoryReset();
  }

  for(i=0; i < heaterCount; i++) {	// set point
    heaters[i].factoryReset();
  }

  ResetFunction();
}


