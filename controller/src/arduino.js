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
    //    Note - this routine used to split this into a write of the
    //    register followed by a read. I believe this was the source
    //    of errors because another thread Promise could be executed
    //    between the write of the register and the read, potentially
    //    causing the wrong data to be read back.
    //
    //    So I switched to another call in the i2c-bus library that
    //    combined the write & read together. This means that another
    //    Promise can't get in there. This call was specifically meant
    //    for the SMBus, which looks to be quite compatible.
    //
    readBytes(register,count)
    {
	var rbuf = Buffer.alloc(count);
	return(
	    this.i2c.readI2cBlock(ARDUINO_ADDR,register,count,rbuf)
		.then(() => rbuf)
	);
    }
}
    
module.exports = 
    i2c.openPromisified(busNum)
    .then((i2cObj) => new Arduino(i2cObj));
