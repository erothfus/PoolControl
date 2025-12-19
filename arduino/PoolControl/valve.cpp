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
#include <Arduino.h>
#include "valve.h"
#include <EEPROM.h>

#define DEFAULT_TRAVEL_DIR  0
#define DEFAULT_MIN_DEG     0
#define DEFAULT_MAX_DEG     180

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
Valve::Valve(int onPin, int dirPin, int monitorPin, int eepromAddress)
{
  pinON = onPin;
  pinDIR = dirPin;
  pinMONITOR = monitorPin;

  myAddress = eepromAddress;

  degMIN = DEFAULT_MIN_DEG;
  degMAX = DEFAULT_MAX_DEG;
  degNOW = degMIN;

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
void Valve::config(int min, int max)
{
  if(min < max) {
    degMIN = min;
    degMAX = max;
  } else {
    degMIN = max;
    degMAX = min;
  }
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
    relayControl(pinDIR,DIR_NEGATIVE);		// run negative for 1 second
    relayControl(pinON,RELAY_ON);
    Serial.println("one second negative");
    stateSwitch(ValveStates::CALIBRATE_START3,1000000UL);
    break;

  case ValveStates::CALIBRATE_START3:
    relayControl(pinDIR,DIR_POSITIVE);
    relayControl(pinON,RELAY_ON);		// run positive for 1 second
    Serial.println("one second positive");
    stateSwitch(ValveStates::CALIBRATE_START4,1000000UL);
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
    degNOW = degMIN;
    Serial.println("LIMIT DONE");
    relayControl(pinON,RELAY_OFF);
    neg_time = micros() - neg_time;
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

void Valve::movementLoop()
{
  int tolerance = currentBenchmark / 5;		// 20% tolerance - seems high...
  int lowBracket = currentBenchmark - tolerance;
  int highBracket = currentBenchmark + tolerance;
  int current = readCurrent();
  unsigned long targetTime;	// set to the time for positioned moves

#define USING_CURRENT (current >= highBracket || current <= lowBracket)

  switch(state_current) {

  // Move_LIMIT_LOW - move to lower limit, without worrying about much
    
  case ValveStates::MOVE_LIMIT_LOW:
    relayControl(pinON,RELAY_ON);
    relayControl(pinDIR,DIR_NEGATIVE);
    stateSwitch(ValveStates::MOVE_LIMIT_LOW2,1000000UL); // 1 second start
    break;

  case ValveStates::MOVE_LIMIT_LOW2:
    if(!USING_CURRENT) {
      // when we see an inactive current, attempt to read it
      //   for 100ms, or come back here
      stateSwitch(ValveStates::MOVE_LIMIT_LOW3);
    }
    break;

  case ValveStates::MOVE_LIMIT_LOW3:
    // wait for 100ms to see if we are ACTUALLY settled, by
    //   letting this state timeout
    stateTimeout(ValveStates::MOVE_LIMIT_LOW_DONE,100000UL);

    // if we see something OTHER than inactive, switch back to waiting
    if(USING_CURRENT) {
      stateSwitch(ValveStates::MOVE_LIMIT_LOW2);
    }
    break;

  case ValveStates::MOVE_LIMIT_LOW_DONE:
    degNOW = degMIN;
    relayControl(pinON,RELAY_OFF);
    stateSwitch(ValveStates::INACTIVE);
    break;
  
  // Move_LIMIT_HIGH - move to high limit, without worrying about much
    
  case ValveStates::MOVE_LIMIT_HIGH:
    relayControl(pinON,RELAY_ON);
    relayControl(pinDIR,DIR_POSITIVE);
    stateSwitch(ValveStates::MOVE_LIMIT_HIGH2,1000000UL); // 1 second start
    break;

  case ValveStates::MOVE_LIMIT_HIGH2:
    if(!USING_CURRENT) {
      // when we see an inactive current, attempt to read it
      //   for 100ms, or come back here
      stateSwitch(ValveStates::MOVE_LIMIT_HIGH3);
    }
    break;

  case ValveStates::MOVE_LIMIT_HIGH3:
    // wait for 100ms to see if we are ACTUALLY settled, by
    //   letting this state timeout
    stateTimeout(ValveStates::MOVE_LIMIT_HIGH_DONE,100000UL);

    // if we see something OTHER than inactive, switch back to waiting
    if(USING_CURRENT) {
      stateSwitch(ValveStates::MOVE_LIMIT_HIGH2);
    }
    break;

  case ValveStates::MOVE_LIMIT_HIGH_DONE:
    degNOW = degMAX;
    relayControl(pinON,RELAY_OFF);
    stateSwitch(ValveStates::INACTIVE);
    break;

  // move to the degTARGET configured when called - this will take
  //   move in the right direction depending upon where we currently
  //   are.

  case ValveStates::MOVE_TARGET:
    if(degNOW == degTARGET) {
      stateSwitch(ValveStates::INACTIVE);
      break;
    }

    if(degNOW < degTARGET) {
      relayControl(pinDIR,DIR_POSITIVE);
      targetTime =
	pos_time / (unsigned long)(degMAX - degMIN) * (unsigned long)(degTARGET - degNOW);
    } else {
      relayControl(pinDIR,DIR_NEGATIVE);
      targetTime = 
	neg_time / (unsigned long)(degMAX - degMIN) * (unsigned long)(degNOW - degTARGET);
    }
    relayControl(pinON,RELAY_ON);

    stateSwitch(ValveStates::MOVE_TARGET_DONE,targetTime);
    break;

  case ValveStates::MOVE_TARGET_DONE:
    relayControl(pinON,RELAY_OFF);
    degNOW = degTARGET;
    stateSwitch(ValveStates::INACTIVE);
    break;
  }
}
  
		 

  
