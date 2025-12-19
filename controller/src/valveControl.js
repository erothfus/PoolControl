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
	Arduino.writeBytes(command,sendArray.length,sendArray);
	return('OK - moving to ' + degrees);
    }
	

    status()
    {
	var register = this.valveNum;
	return(
	    Arduino.readBytes(register,4)
		.then((data) => {
//		    console.log("status",data);
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
		    console.log("travel",data);
		    return({pos:(data[0]<<8)+data[1],neg:(data[2]<<8)+data[3]});
		})
	);
    }
	
}
