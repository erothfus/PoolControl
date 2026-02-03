//
// valve.h
//
//   (see valve.cpp for information about this class)
//

#ifndef VALVE_H
#define VALVE_H

#include <time.h>
#include <Arduino.h>
#include "eeprom.h"

// ValveStates defines all of the states that a valve can be in, which
//  drives the different sub-state-machines for a valve - like "calibration"
//  and "movement"

enum class ValveStates { 

  INACTIVE = 0,			// the valve is not doing anything right now
  SEEK_FAIL = 1,			// failure during a move to a position
  // ...

  // CALIBRATION STATES
  CALIBRATE_START = 100,		// initial state that kicks-off calibration
  CALIBRATE_START2= 120,		// initial state that kicks-off calibration
  CALIBRATE_START3= 121,		// initial state that kicks-off calibration
  CALIBRATE_START4= 122,		// initial state that kicks-off calibration
  // Phase 1
  CALIBRATE_BENCHMARK = 101,		// the state where valve current is benchmarked
  CALIBRATE_INITIATE = 102,		// calibration starts for the valve
  CALIBRATE_LIMITSEEK1 = 103,	
  CALIBRATE_LIMITSEEK2 = 104,	
  CALIBRATE_LIMIT = 105,		// first limit hit
  // Phase 2
  CALIBRATE_BENCHMARK2 = 106,		// the state where valve current is bjenchmarked
  CALIBRATE_INITIATE2 = 107,		// calibration starts for the valve
  CALIBRATE_LIMITSEEK21 = 108,	
  CALIBRATE_LIMITSEEK22 = 109,	
  CALIBRATE_LIMIT2 = 110,		// first limit hit
  // Phase 3
  CALIBRATE_BENCHMARK3 = 111,		// the state where valve current is benchmarked
  CALIBRATE_INITIATE3 = 112,		// calibration starts for the valve
  CALIBRATE_LIMITSEEK31 = 113,	
  CALIBRATE_LIMITSEEK32 = 114,	
  CALIBRATE_LIMIT3 = 115,		// first limit hit
  
  CALIBRATE_END = 150,		// calibrate end-state - used really as a marker

  // MOVEMENT STATES

  MOVE_LIMIT_LOW = 200,		// beginning state for a move to lower limit
  MOVE_LIMIT_LOW2 = 201,
  MOVE_LIMIT_LOW3 = 202,
  MOVE_LIMIT_LOW_DONE = 203,

  MOVE_LIMIT_HIGH = 230,	// beginning state for a move to upper limit
  MOVE_LIMIT_HIGH2 = 231,
  MOVE_LIMIT_HIGH3 = 232,
  MOVE_LIMIT_HIGH_DONE = 233,

  MOVE_TARGET = 210,		// beginning state for a move to position
  MOVE_TARGET_DONE = 211

};

#define STATE_MACHINE
#define STATE_TIMEOUT

class Valve : public EEPROM_CONTROL {

public:
  Valve(int,int,int,int);
  void configTravelLimits(int,int);
  void configTravelTimes(unsigned long,unsigned long);
  void configPosition(int);
  void loop();

  void status(byte *);	// fills in the byte array [0] current state, [1] prev [2-3] position

  void degreesGet(int, byte *);	// gets the curent configured degrees for valve
  void travelTime(byte *);	// gets the current calibrated travel time

  void calibrate();
  int calibrateStatus();

  void move(int degrees);
  int moveStatus();
  
private:
  int pinON;		// digital pin that controls the valve "on" relay
  int pinDIR; 		// digital pin that controls the valve "direction" relay
  int pinMONITOR;	// analog pin that monitors the valve
  int travelDIR;	// definition of the travel direction = 0 or 1
                        //   where 0 corresponds to the degMIN stop. That is
                        //   setting 0 moves the valve toward degMIN tsop.
  int degMIN;		// degrees assigned to minimum stop
  int degMAX;		// degrees assigned to max stop
  int degNOW;		// current degrees (may be estimated)
  int degTARGET;	// set when doing a positioned movement

  int currentBenchmark;	// tracks the measured inactive current

  void loadTravelLimits(void);
  void loadTravelTimes(void);
  void loadPosition(void);

  void calibrationLoop();	// called by loop() - for calibration operation
  int calibrationComplete();	// returns true when the calibration is complete
  void movementLoop();		// called by loop() - for movement operation
  int movementComplete();	// returns true when the movement is complete
  void relayControl(int,int);   // turns on the given relay pin
  int readCurrent(void);	// reads the valve current sensor

  // state maintenance members
  unsigned long	state_delay;	// micros to delay before the next state
  ValveStates	state_next;	//   what that next state is (only valid after delay)

  unsigned long	state_timeout;	// micros to wait for state to change
  ValveStates	state_failure;	//   switch-to state upon timeout failure

  ValveStates	state_current;
  ValveStates	state_prev;

  unsigned long pos_time;	// time to go from 0 to upper limit
  unsigned long neg_time;	// time to go from upper limit to 0
  
  unsigned long	state_lastSwitch;	// micros at last state switch

  int stateUpdate(void);
  void stateSwitch(ValveStates);
  void stateSwitch(ValveStates,unsigned long);
  void stateTimeout(ValveStates,unsigned long);

};

#endif // VALVE_H
