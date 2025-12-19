//
// arduino.js
//
//   Interface to the Arduino. Used for valves and termometers.
//
const i2c = require('i2c-bus');

ARDUINO_ADDR = 0x20;
busNum = 1;

class Arduino {

    constructor(i2cObj)
    {
	this.i2c = i2cObj;
    }

    writeByte(register,byte)
    {
	return(this.i2c.writeByte(ARDUINO_ADDR,register,byte));
    }

    readByte(register)
    {
	return(
	    this.i2c.readByte(ARDUINO_ADDR,register)
		.catch((e) => { console.log("readByte() error (ignored)",e);
				return(0);
			      })
	);
    }

    //
    // writeBytes() - write more than one byte (along with the register)
    //    to the Arduino.
    //
    writeBytes(register,count,bytes)
    {
	var wbuf = Buffer.from([register,...bytes]);
	return(this.i2c.i2cWrite(ARDUINO_ADDR,count+1,wbuf));
    }

    //
    // readBytes() - read the available bytes from the given i2c
    //    register query. Will read as many as available and return
    //    a Buffer with the data.
    //
    readBytes(register,count)
    {
	var wbuf = Buffer.from([register]);
	var rbuf = Buffer.alloc(count);
	return(
	    this.i2c.i2cWrite(ARDUINO_ADDR,1,wbuf)
		.then(() => this.i2c.i2cRead(ARDUINO_ADDR,count,rbuf))
		.then(() => rbuf)
	);
    }
}
    
module.exports = 
    i2c.openPromisified(busNum)
    .then((i2cObj) => new Arduino(i2cObj));
