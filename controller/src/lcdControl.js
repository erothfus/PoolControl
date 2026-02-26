//
// lcdControl.js
//
//   Used to interface the the LCD attached to the RPi via i2c.
//
//   The LCD display is fully managed by this routine, so that
//   the rest of the code just needs to supply data, and it is
//   displayed as dictated by this code.

const LCD = require('raspberrypi-liquid-crystal');
var lcd;

const IP = require('./ipAddress');

//
// init() - initializes the communication to the LCD
//
function init(bus)
{                 // bus  i2c-id  cols  rows
    lcd = new LCD(   bus,  0x27,   16,   2);
    
    return(displayStart());
}

//
// displayStart() - starts the display loop
//
async function displayStart()
{
    setInterval(screen1,20000);
    setTimeout(()=>setInterval(screen2,20000),10000);
}

//
// screen1() - the first screen in the display
//
function screen1()
{
    return(
	lcd.begin()
	    .then(() =>	lcd.clear())
	    .then(() => lcd.printLine(0,IP.wlan0[0]))
	    .then(() => lcd.close())
    );
}

//
// screen2() - the second screen in the display
//
function screen2()
{
    return(
	lcd.begin()
	    .then(() =>	lcd.clear())
	    .then(() => lcd.printLine(0,"Hello World"))
	    .then(() => lcd.close())
    );
}



module.exports = {
    init,
};
