//
// valve.cpp
//
//   This file implements valve control.
//
//   The class "Valve" has the following methods:
//
//     (constructor) - given two digital pins, one for ON and the
//        other for DIRECTION, along with an analog pin for MONITOR,
//        create a structure allowing the control of a pool valve.
//
//   NOTE - we talk directly to the relays here, and the sense of the
//   relays is given as RELAY_ON/_OFF. Use those defines for relay control.
//
//   There are 3 things that get stored in EEPROM for valves:
//
//     - the width of travel for the valve is not really implemented
//       right now - but it is there and gets stored
//
//     - the current position of the valve is always put into EEPROM
//       so that a power fail doesn't forget where the valve is
//
//     - the calibrated travel time (clockwise and counter)
//

#include <Arduino.h>
#include "valve.h"
#include <EEPROM.h>

// Default values for EEPROM-stored data

#define DEFAULT_TRAVEL_DIR  0
#define DEFAULT_MIN_DEG     0
#define DEFAULT_MAX_DEG     180
#define DEFAULT_UP_TIME	    5000000L
#define DEFAULT_DOWN_TIME    5000000L
#define DEFAULT_POSITION    0

// Since valves stored multiple categories of data, the config/store
//   routines need different offsets - one after the other

#define TRAVEL_TIMES_OFFSET	0
#define TRAVEL_TIMES_SIZE	(sizeof(unsigned long)*2)
#define TRAVEL_LIMITS_OFFSET	(TRAVEL_TIMES_OFFSET + TRAVEL_TIMES_SIZE)
#define TRAVEL_LIMITS_SIZE	(sizeof(int)*2)
#define POSITION_OFFSET		(TRAVEL_LIMITS_OFFSET + TRAVEL_LIMITS_SIZE)

// go ahead and adjust these two values if the relays change
#define RELAY_ON	LOW
#define RELAY_OFF	HIGH

// valves start at 0 degrees and move up from there, and we
//   simply DEFINE positive movement as "on" for the relay
#define DIR_POSITIVE	RELAY_ON
#define DIR_NEGATIVE	RELAY_OFF

//
// status() - returns the status of the valve in 4 bytes:
//   byte 1 - current state machine state
//   byte 2 - previous state machine state
//   byte 3 - upper byte of position
//   byte 4 - lower byte of position
//
void Valve::status(byte *buffer)
{
  buffer[0] = (byte)state_current;
  buffer[1] = (byte)state_prev;
  buffer[2] = (byte)((degNOW >> 8)&0xff);
  buffer[3] = (byte)(degNOW &0xff);
}

//
// degrees() - returns the min or max degrees configured for
//    this valve in 2 bytes - big endian. If minMax is true (1)
//    then send back max, otherwise min.
//   byte 0 - upper byte of position
//   byte 1 - lower byte of position
//
void Valve::degreesGet(int minMax,byte *buffer)
{
  int degrees = degMIN;
  if(minMax) {
    degrees = degMAX;
  }

  buffer[0] = (byte)((degrees >> 8)&0xff);
  buffer[1] = (byte)(degrees &0xff);
}

//
// travelTime() - returns the positive and negative travel time
//    in tenths of seconds (to fit in a 2-byte int, and because
//    anything else really isn't interesting). EG - 241 means
//    24.1 seconds.
//    
void Valve::travelTime(byte *buffer)
{
  int pos = pos_time/100000;
  int neg = neg_time/100000;
  
  buffer[0] = (byte)((pos >> 8)&0xff);
  buffer[1] = (byte)(pos & 0xff);
  buffer[2] = (byte)((neg >> 8)&0xff);
  buffer[3] = (byte)(neg & 0xff);
}
  

void Valve::stateSwitch(ValveStates newState)
{
  stateSwitch(newState,0UL);
}

void Valve::stateSwitch(ValveStates newState, unsigned long delay)
{
  //  Serial.print("->"); Serial.println((int)newState);
  state_next = newState;
  state_delay = delay;
}

void Valve::stateTimeout(ValveStates newState, unsigned long timeout)
{
  state_failure = newState;
  state_timeout = timeout;
}

//
// ::stateUpdate() - checks for delays and timeouts, updating the
//    state if appropriate.
//    Returns 1/true if the state machine chould continue, or
//    0/false if not. This is used so that states issueing a
//    "next state" don't continue to process.
//
int Valve::stateUpdate()
{
  unsigned long currentMicros = micros();
  unsigned long since = currentMicros - state_lastSwitch;

  //  Serial.print("Since:");
  //  Serial.println(since);
  
  //  Serial.print("Delay:");
  //  Serial.println((unsigned long)state_delay);
  
  if(state_delay != 0UL && since < state_delay) {// move to next state if it's time
    return(false);				//   otherwise indicate waiting
  }

  state_delay = 0UL;

  // first, are we looking to switch states
  if(state_next != state_current) {

    state_prev = state_current;
    state_current = state_next;
    state_delay = 0;
    state_timeout = 0;
    state_lastSwitch = currentMicros;

  } else {
    // no state change requested, check timeout
    if(state_timeout > 0 && since >= state_timeout) {
      state_prev = state_current;
      state_current = state_failure;
      state_delay = 0;
      state_timeout = 0;
      state_lastSwitch = currentMicros;
    }
  }

  return(true);
  
}

//
// relayControl() - controls the given relay where:
//    relayPin - set to tiehter pinON or pinDIR for that relay
//    onoff - if TRUE turn on the relay, otherwise turn it off
//
void Valve::relayControl(int relayPin, int onoff)
{
  digitalWrite(relayPin,onoff);
}

//
// readCurrent() - a little micro-history here. My original thought
//    was that the current readings weren't stable, that is, that they
//    would jump around for some reason. I think this was ultrasonic
//    transducer experience poluting me. Because of this, I thought
//    I would need to average the readings over time. I'm not doing
//    that any more.
//
//    This routine, btw, returns a value between 0 and 1023 indicating
//    the voltage from 0 to 5v. The current sensor appears to read
//    forward & reverse current, with one between 0 and 2.5v and the
//    other between 2.5 and 5v.
//
int Valve::readCurrent(void)
{
  int current = analogRead(pinMONITOR);// between 0 and 1023 inclusive;

  return(current);
}

//
// Valve() - (constructor) Creates a new valve that can be controlled.
//    ARGS:
//       onPin - the digital pin controlling the "on" relay
//       dirPin - the digital pin controlling the "direction" relay
//       monitorPin - the analog pin monitoring valve movement
//       min - the degrees where the minimum stop is
//       max - the degrees where the maximum stop is (should be bigger than min)
//      
Valve::Valve(int onPin, int dirPin, int monitorPin, int eepromAddress) : EEPROM_CONTROL(eepromAddress)
{
  pinON = onPin;
  pinDIR = dirPin;
  pinMONITOR = monitorPin;

  if(!eepromHasBeenSet()) {
    configTravelTimes(DEFAULT_UP_TIME,DEFAULT_DOWN_TIME);
    configTravelLimits(DEFAULT_MIN_DEG,DEFAULT_MAX_DEG);
    configPosition(DEFAULT_POSITION);
  } else {
    loadTravelTimes();
    loadTravelLimits();
    loadPosition();
  }

  relayControl(pinON,RELAY_OFF);	// done before setting as output
  relayControl(pinDIR,RELAY_OFF);

  pinMode(pinON,OUTPUT);
  pinMode(pinDIR,OUTPUT);

  state_current = ValveStates::INACTIVE;
  state_next = ValveStates::INACTIVE;
  state_prev = ValveStates::INACTIVE;
  state_timeout = 0;
  state_delay = 0;

  currentBenchmark = 0;		// useless default
}

//
// config() - configure this valve with the appropriate settings for movement.
//
void Valve::configTravelLimits(int min, int max)
{
  int offset = TRAVEL_LIMITS_OFFSET;

  if(min < max) {
    degMIN = min;
    degMAX = max;
  } else {
    degMIN = max;
    degMAX = min;
  }

  offset += eepromWrite(offset,degMIN);
  offset += eepromWrite(offset,degMAX);
}
void Valve::loadTravelLimits()
{
  int offset = TRAVEL_LIMITS_OFFSET;

  offset += eepromRead(offset,&degMIN);
  offset += eepromRead(offset,&degMAX);
}

void Valve::configTravelTimes(unsigned long up, unsigned long down)
{
  int offset = TRAVEL_TIMES_OFFSET;

  pos_time = up;
  neg_time = down;

  offset += eepromWrite(offset,pos_time);
  offset += eepromWrite(offset,neg_time);
}
void Valve::loadTravelTimes()
{
  int offset = TRAVEL_TIMES_OFFSET;

  offset += eepromRead(offset,&pos_time);
  offset += eepromRead(offset,&neg_time);
}

void Valve::configPosition(int pos)
{
  int offset = POSITION_OFFSET;

  degNOW = pos;

  offset += eepromWrite(offset,degNOW);
}
void Valve::loadPosition()
{
  int offset = POSITION_OFFSET;

  offset += eepromRead(offset,&degNOW);
}

//
// loop() - called by the main routine(s) to cause the valve to continue
//    to operate. For example, if it is in the middle of a calibration,
//    the loop() routine being called repeatedly allows the calibration
//    to complete. This should be called quite often.
//
// calibrate states
// - turn off all relays, and wait for 1/2 second
// - take current reading, setting that as "quiescent" - turn on CW and power relays - wait 1/10 sec
//  - note that the motor takes time for power-up, so the current reading should be delayed
//- take current reading, if within 10% quiescent - next state - other wise repeat

void Valve::loop()
{
  if(stateUpdate()) {	// updates state as needed, false if waiting
    calibrationLoop();
    movementLoop();
  }
}

//
// calibrate() - used to "calibrate" a valve by running it to one stop,
//    then to the other, measuring the time it takes to get to the
//    second stop from the first. For this version, we are assuming
//    that the motor has the same speed in getting from the first
//    stop to the second.
//
//    Note that it can take many seconds to do a full travel, so the
//    MS can be (probably) in the tens of thousands, which is just fine
//    for an int.
//
//    This routine uses its own state machine to maintain its operation
//    when the calibrationLoop() is repeatedly called.
//
void Valve::calibrate()
{
  stateSwitch(ValveStates::CALIBRATE_START);
}

int Valve::calibrationComplete()
{
  return(1);
}

//
// calibrationLoop() - (see calibrate() for information on the process)
//      Needs to be called repeatedly/quickly to maintain the process
//      of calibration. The state machine for calibration is maintained
//      and updated here.
//
//      You *could* call this only when in a calibration process, but
//      since it only reacts to calibration states, you can call it any
//      time really.
//
void Valve::calibrationLoop()
{
  // potentially used later, takes just a bit of time
  //   to set them up now - putting them in the case statement breaks it!

  // BIG NOTE - the calibrationLoop() runs continuously (obviously) which
  //   means that by the time it is needed, the currentBenchmark has been
  //   set, allowing appropriate usage of the three derivative measures.
  
  int tolerance = currentBenchmark / 5;		// 20% tolerance - seems high...
  int lowBracket = currentBenchmark - tolerance;
  int highBracket = currentBenchmark + tolerance;
  
  int current = readCurrent();

#define USING_CURRENT (current >= highBracket || current <= lowBracket)
  
  //  Serial.print("::"); Serial.print((int)state_current); Serial.println("::");
  
  switch(state_current) {
    
  case ValveStates::CALIBRATE_START:
    relayControl(pinON,RELAY_OFF);	// ensure off for .1 seconds
    Serial.println("Starting");
    stateSwitch(ValveStates::CALIBRATE_START2,100000UL);
    break;

  case ValveStates::CALIBRATE_START2:
    relayControl(pinDIR,DIR_NEGATIVE);		// run negative for 3 seconds
    relayControl(pinON,RELAY_ON);
    Serial.println("one second negative");
    stateSwitch(ValveStates::CALIBRATE_START3,3000000UL);
    break;

  case ValveStates::CALIBRATE_START3:
    relayControl(pinDIR,DIR_POSITIVE);
    relayControl(pinON,RELAY_ON);		// run positive for 3 second
    Serial.println("one second positive");
    stateSwitch(ValveStates::CALIBRATE_START4,3000000UL);
    break;

  case ValveStates::CALIBRATE_START4:
    relayControl(pinON,RELAY_OFF);		// ensure off for .1 seconds
    stateSwitch(ValveStates::CALIBRATE_BENCHMARK,100000UL);
    break;

  // PHASE I - get to the lower limit
    
  case ValveStates::CALIBRATE_BENCHMARK:
    currentBenchmark = current;
    Serial.print("Current:");
    Serial.println(currentBenchmark);
    stateSwitch(ValveStates::CALIBRATE_INITIATE);
    break;

  case ValveStates::CALIBRATE_INITIATE:
    relayControl(pinDIR,DIR_NEGATIVE);
    relayControl(pinON,RELAY_ON);
    stateSwitch(ValveStates::CALIBRATE_LIMITSEEK1,1000000UL); // 1s to spin-up
    break;

  case ValveStates::CALIBRATE_LIMITSEEK1:
    if(!USING_CURRENT) {
      // when we see an inactive current, attempt to read it
      //   for 100ms, or come back here
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK2);
    }
    break;

  case ValveStates::CALIBRATE_LIMITSEEK2:
    // wait for 100ms to see if we are ACTUALLY settled, by
    //   letting this state timeout
    stateTimeout(ValveStates::CALIBRATE_LIMIT,100000UL);

    // if we see something OTHER than inactive, switch back to waiting
    if(USING_CURRENT) {
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK1);
    }
    break;
    
  case ValveStates::CALIBRATE_LIMIT:
    Serial.println("LIMIT");
    relayControl(pinON,RELAY_OFF);
    stateSwitch(ValveStates::CALIBRATE_BENCHMARK2,100000UL);
    break;

  // PHASE II - get to the positive limit, timing it
    
  case ValveStates::CALIBRATE_BENCHMARK2:
    currentBenchmark = current;
    Serial.print("Current:");
    Serial.println(currentBenchmark);
    stateSwitch(ValveStates::CALIBRATE_INITIATE2);
    break;

  case ValveStates::CALIBRATE_INITIATE2:
    pos_time = micros();
    relayControl(pinDIR,DIR_POSITIVE);
    relayControl(pinON,RELAY_ON);
    stateSwitch(ValveStates::CALIBRATE_LIMITSEEK21,1000000UL); // 1s to spin-up
    break;

  case ValveStates::CALIBRATE_LIMITSEEK21:
    if(!USING_CURRENT) {
      // when we see an inactive current, attempt to read it
      //   for 100ms, or come back here
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK22);
    }
    break;

  case ValveStates::CALIBRATE_LIMITSEEK22:
    // wait for 100ms to see if we are ACTUALLY settled, by
    //   letting this state timeout
    stateTimeout(ValveStates::CALIBRATE_LIMIT2,100000UL);

    // if we see something OTHER than inactive, switch back to waiting
    if(USING_CURRENT) {
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK21);
    }
    break;
    
  case ValveStates::CALIBRATE_LIMIT2:
    Serial.println("LIMIT");
    relayControl(pinON,RELAY_OFF);
    pos_time = micros() - pos_time;
    degNOW = degMAX;		// just for illustration - doesn't play a role here
    stateSwitch(ValveStates::CALIBRATE_BENCHMARK3);
    break;

  // PHASE III - go (back) to the lower limit
    
  case ValveStates::CALIBRATE_BENCHMARK3:
    currentBenchmark = current;
    Serial.print("Current:");
    Serial.println(currentBenchmark);
    stateSwitch(ValveStates::CALIBRATE_INITIATE3,100000UL);
    break;

  case ValveStates::CALIBRATE_INITIATE3:
    neg_time = micros();
    relayControl(pinDIR,DIR_NEGATIVE);
    relayControl(pinON,RELAY_ON);
    stateSwitch(ValveStates::CALIBRATE_LIMITSEEK31,1000000UL); // 1s to spin-up
    break;

  case ValveStates::CALIBRATE_LIMITSEEK31:
    if(!USING_CURRENT) {
      // when we see an inactive current, attempt to read it
      //   for 100ms, or come back herej
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK32);
    }
    break;

  case ValveStates::CALIBRATE_LIMITSEEK32:
    // wait for 100ms to see if we are ACTUALLY settled, by
    //   letting this state timeout
    stateTimeout(ValveStates::CALIBRATE_LIMIT3,100000UL);

    // if we see something OTHER than inactive, switch back to waiting
    if(USING_CURRENT) {
      stateSwitch(ValveStates::CALIBRATE_LIMITSEEK31);
    }
    break;
    
  case ValveStates::CALIBRATE_LIMIT3:
    configPosition(degMIN);
    Serial.println("LIMIT DONE");
    relayControl(pinON,RELAY_OFF);
    configTravelTimes(pos_time,micros() - neg_time);
    Serial.print("POS:");
    Serial.println(pos_time);
    Serial.print("NEG:");
    Serial.println(neg_time);
    stateSwitch(ValveStates::INACTIVE);
    break;

  default:
    break;
  }
}

//
// move() - given a target degrees, move there.
//   The limits are special cased, in that they can be used
//   to reset the positioning accuracy.
//
void Valve::move(int target)
{
  degTARGET = target;
  if(target == degMIN) {
    stateSwitch(ValveStates::MOVE_LIMIT_LOW);
  } else if(target == degMAX) {
    stateSwitch(ValveStates::MOVE_LIMIT_HIGH);
  } else {
    stateSwitch(ValveStates::MOVE_TARGET);
  }
}

int Valve::movementComplete()
{
  return(1);
}

//
// movementLoop() - this is called from the mail Arduino app loop, and
//    controls valve movement (as opposed to calibration) - this is done
//    are more often than calibration.
//
//    Movement to points other than the limits is done based upon the
//    calibrated time.
//
//    NOTE - at one point this routine tried to use the current sensor
//    when going to the limits, as opposed to time - HOWEVER, there
//    wasn't a simple way to do that due to the sensor value changing
//    over time as the capacitor in the valve drains. So, instead,
//    this routine just uses the time PLUS a bit to ensure that it hits
//    the limits when going to them.
//
void Valve::movementLoop()
{
    switch(state_current) {

	// note that we can get in to this routine from the "old"
	//   entry points, too, that used to differentiate between
	//   limit movement and target movement. Now they just
	//   initiate a target movement.
	
    case ValveStates::MOVE_LIMIT_LOW:
	degTARGET = degMIN;
	stateSwitch(ValveStates::MOVE_TARGET);
	break;
	
    case ValveStates::MOVE_LIMIT_HIGH:
	degTARGET = degMAX;
	stateSwitch(ValveStates::MOVE_TARGET);
	break;
	
    case ValveStates::MOVE_TARGET:

	// do nothing if we're already at the right target
    
	if(degNOW == degTARGET) {
	    stateSwitch(ValveStates::INACTIVE);
	    break;
	}

	// now set the time that we need to move based upon where we are

	if(degNOW < degTARGET) {
	    targetTime = pos_time / (unsigned long)(degMAX - degMIN) * (unsigned long)(degTARGET - degNOW);
	} else {
	    targetTime = neg_time / (unsigned long)(degMAX - degMIN) * (unsigned long)(degNOW - degTARGET);
	}

	// if we're moving to a limit, add a bit of movement time to take
	//   care of any slop that has accumulated since we're not using
	//   current sensing (see main comments)
    
	if(degTARGET == degMAX || degTARGET == degMIN) {
	    targetTime += 2000000UL;	// 2 second additional movement for limits
	}

	// now, split up the time into 6 segments to allow feedback to go back to the user
	targetTime /= 6;
	
	relayControl(pinDIR,(degNOW < degTARGET)?DIR_POSITIVE:DIR_NEGATIVE);	
	relayControl(pinON,RELAY_ON);
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_1);
	break;

	// the whole purpose of the next state is to provide periodic
	//   feedback to the user - we cut the movement into a few
	//   segments, reporting for each segment
	
    case ValveStates::MOVE_TARGET_PROCESS_1:
	degNOW += (degTARGET - degNOW)/6;
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_2,targetTime);
	break;

    case ValveStates::MOVE_TARGET_PROCESS_2:
	configPosition(degNOW);
	degNOW += (degTARGET - degNOW)/5;
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_3,targetTime);
	break;

    case ValveStates::MOVE_TARGET_PROCESS_3:
	configPosition(degNOW);
	degNOW += (degTARGET - degNOW)/4;
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_4,targetTime);
	break;

    case ValveStates::MOVE_TARGET_PROCESS_4:
	configPosition(degNOW);
	degNOW += (degTARGET - degNOW)/3;
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_5,targetTime);
	break;

    case ValveStates::MOVE_TARGET_PROCESS_5:
	configPosition(degNOW);
	degNOW += (degTARGET - degNOW)/3;
	stateSwitch(ValveStates::MOVE_TARGET_PROCESS_6,targetTime);
	break;

    case ValveStates::MOVE_TARGET_PROCESS_6:
	configPosition(degNOW);
	degNOW += (degTARGET - degNOW)/2;
	stateSwitch(ValveStates::MOVE_TARGET_DONE,targetTime);
	break;

    case ValveStates::MOVE_TARGET_DONE:
	degNOW = degTARGET;		// make sure we're RIGHT on
	configPosition(degNOW);
	relayControl(pinON,RELAY_OFF);
	stateSwitch(ValveStates::INACTIVE);
	break;
    }
}
  
		 

  
