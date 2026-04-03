//
// lcdControl.js
//
//   Used to interface the the LCD attached to the RPi via i2c.
//
//   The LCD display is fully managed by this routine, so that
//   the rest of the code just needs to supply data, and it is
//   displayed as dictated by this code.
//
//   There appears to be some bug, over time, where the display
//   get scrambled. My thought was that the Arduino and the LCD
//   were using two different instantiations of the i2c bus code,
//   which was causing contention.
//
//   The plan was to use the same code, so this routine now gets
//   passed the i2c object when being instantiated. HOWEVER, I
//   haven't converted the code to be able to USE the object.
//
//   Instead, it still uses the raspberrypi-liquid-crystal module
//   which re-uses the i2c module.
//
const IP = require('./ipAddress');
const LCD = require('raspberrypi-liquid-crystal');
var lcd;

class LCDControl {

    //
    // constructor() - as described in the comment above, this routine
    //    doesn't yet use the passed-in ic2obj - like it should.
    //
    constructor(i2cObj,bus)
    {
	this.i2c = i2cObj;   // CURRENTLY NOT USED :-(
	this.lcd = new LCD(   bus,  0x27,   16,   2);
    }

    //
    // displayStart() - starts the display loop
    //
    async displayStart()
    {
	var speed = 2000;
	
	setInterval(this.screen1.bind(this),speed);
	setTimeout(()=>setInterval(this.screen2.bind(this),speed),speed/2);
    }

    //
    // screen1() - the first screen in the display
    //
    screen1()
    {
	return(
	    this.lcd.begin()
		.then(() => this.lcd.clear())
		.then(() => this.lcd.printLine(0,IP.wlan0[0]))
		.then(() => this.lcd.close())
		.catch((e) => console.log("LCD Error (1) -",e))
	);
    }

    //
    // screen2() - the second screen in the display
    //
    screen2()
    {
	return(
	    this.lcd.begin()
		.then(() => this.lcd.clear())
		.then(() => this.lcd.printLine(0,"Hello World"))
		.then(() => this.lcd.close())
		.catch((e) => console.log("LCD Error (2) -",e))
	);
    }
}

module.exports = LCDControl;
