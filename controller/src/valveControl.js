//
// valveControl.js
//
//   Code for managing valves.
//
//   This module defines an object that is used to control
//   a particular valve.
//
//   Valve control is implemented through the attached Arduino.
//   There are global constants that can be used to reach it
//   through the i2c bus.

const i2c = require('i2c-bus');

const VALVE_INACTIVE = 0;     // defined in the Arduino - seeds public/codes.js too

//
// delayPromise() - little utility function to return a Promise that waits for
//    the given number of seconds.
//
function delayPromise(secs)
{
    return(new Promise((res) => setTimeout(res,secs*1000)));
}

module.exports = class {

    constructor(span,dir,num)
    {
	this.span = span;
	this.dir = dir;
	this.valveNum = num & 0x03;
    }

    calibrate()
    {
	var command = 0x30 + this.valveNum;
	
	Arduino.writeByte(command,0);
	return('Calling calibrate() on ' + this.span + ' ' + this.dir);
    }

    move(degrees)
    {
	var command = 0x50 + this.valveNum;

	var sendArray =  [degrees>>8,degrees&0xff];
	return(
	    Arduino.writeBytes(command,sendArray.length,sendArray)
		.then(() => ({status:'ok'}))
	);
    }

    //
    // secondCheck() - this routine is used when waiting for a valve
    //     to get back to quiescent. It returns a Promise, that
    //     actually refers to itself, so that it will continue to loop
    //     around until the valve gets to the right state, or the
    //     number of 1 second loops gets to the maximum.
    //
    secondCheck(maxWaitSecs)
    {
	return(
	    this.status()
		.then((status) => {
		    if(status.state == VALVE_INACTIVE) {
			return(true);
		    }
		    if(maxWaitSecs == 0) {
			throw false;
		    }
		    return(
			delayPromise(1).then(() => this.secondCheck(maxWaitSecs-1))
		    );
		})
	);
    }
	
    //
    // wait() - waits until this valve has stopped moving (is
    //   quiescent).  This is used primarily when switching modes,
    //   because the pump shouldn't come on before the valve gets done
    //   moving.
    //
    //   NOTE - this used to be part of a "moveWait()" - but that caused
    //   i2c crashing because both valves would enter into moveWait() at
    //   the same time, so their results got scrambled.
    //
    //   The approach here is to check the status of the valve periodically
    //   to see if it has reached the quiescent state. We only wait so
    //   long, however, because if the wait grows larger than 2 times the
    //   rotational time (actually just the sum of the two rotational times)
    //   then we can assume that something went dramatically wrong, and
    //   we throw an error...which SHOULD stop other processes because
    //   the valve isn't in the right position.
    //
    wait()
    {
	return(
	    // first, get the rotational speed to plan for timeout
	    this.travelTimes()
		.then((times) => (times.pos + times.neg)/10)

	    // now we set-up to wait, while checking every second - noting
	    //   that this routine can throw an error
		.then((maxWaitSecs) => this.secondCheck(maxWaitSecs))
	);
    }

    status()
    {
	var register = this.valveNum;
	return(
	    Arduino.readBytes(register,4)
		.then((data) => {
		    return({state:data[0],prev:data[1],position:(data[2]<<8)+data[3]});
		})
	);
    }

    degrees()
    {
	var minRegister = 0x08 | this.valveNum;
	var maxRegister = 0x0c | this.valveNum;
	return(
	    Arduino.readBytes(minRegister,2)
		.then((data) => {
		    var min = (data[0]<<8)+data[1];
		    return(
			Arduino.readBytes(maxRegister,2)
			    .then((data) => {
				var max = (data[0]<<8)+data[1];
				return({min,max});
			    })
		    );
		})
	);
    }

    travelTimes()
    {
	var register = 0x04 | this.valveNum;
	return(
	    Arduino.readBytes(register,4)
		.then((data) => {
//		    console.log("travel",data);
		    return({pos:(data[0]<<8)+data[1],neg:(data[2]<<8)+data[3]});
		})
	);
    }
	
}
