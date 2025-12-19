//
// pumpControl.js
//
//   Code for managing the setting of the pumps.
//
//   This module defines an object that is used to control
//   a particular pump.
//
//   Pump control is implemented through the attached Arduino.
//   There are global constants that can be used to reach it
//   through the i2c bus.
//

const i2c = require('i2c-bus');

module.exports = class {

    constructor(num)
    {
	this.pumpNum = num & 0x03;
    }

    setSpeed(speed)
    {
	var command = 0x70 + ((speed & 0x03) << 2) + this.pumpNum;

	return(
	    Arduino.writeByte(command,0)
		.then((data) => (
		    {status:"ok",
		     operation:`pump ${this.pumpNum} speed ${speed}`}))
	);
    }

    status()
    {
	var command = 0x60 + this.pumpNum;

	return(
	    Arduino.readByte(command)
		.then((data) => {
//		    console.log(`pump status, pump ${this.pumpNum}, ${data}`);
		    return({status:data});
		})
	);
    }
}
	
	
