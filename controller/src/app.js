const UI = require('./ui');
const Valve = require('./valveControl');
const Thermometer = require('./thermometerControl');
const Pump = require('./pumpControl');
const Heater = require('./heaterControl');
const Light = require('./lightControl');
const ArduinoClass = require('./arduino');
const LCDClass = require('./lcdControl');
const Modes = require('./modeControl');
const System = require('./systemControl');

// we get the i2c bus instance here - just once
//   so that everyone that needs it can use the same instance
const i2c = require('i2c-bus');
const i2cBusNum = 1;

// need to read configuration here to create the valves
global.Valves = [
    /* valve 0 */ new Valve(180,'norm',0),
    /* valve 1 */ new Valve(180,'rev',1),
    /* more valves go here - read the config */
];
global.Thermometers = [
    /* therm 0 */ new Thermometer(0),
];

global.Pumps = [
    /* pump 0 - main */ new Pump(0),
    /* pump 1 - booster */ new Pump(1),
];

global.Heaters = [
    /* heater 0 */ new Heater(0),
];

global.Lights = [
    /* light 0 */ new Light(0),
];

global.ModeControl = Modes;

global.SystemControl = new System();

// global catch-all - this is mostly for when the Arduino is
//   being reloaded

process.on("unhandledRejection", (reason) => {
    console.error("Arduino Unresponsive:", reason);
});

async function main() {

    // first, set-up the i2c bus for both the Arduino and display

    i2c.openPromisified(i2cBusNum)
	.then((i2cObj) => {
	    global.Arduino = new ArduinoClass(i2cObj);
	    global.LCD = new LCDClass(i2cObj,i2cBusNum);   // shouldn't have to pass bus num :-(
	})

    // then fire-up the LCD

	.then(() => LCD.displayStart())

    // finally, turn everything off and report the mode

	.then(() => ModeControl.setMode('allOff'))
	.then(() => console.log("allOff executed"))
	.then(() => ModeControl.getSettings())
	.then((settings) => console.log("Settings",settings))
	.then(() => ModeControl.findMode())
	.then((mode) => console.log("Mode is",mode));
}

main();

/******************
if(true) {

} else {
    async function firstTry()
    {
	try {
	    const i2c1 = await i2c.openPromisified(busNum);
	    console.log("I2C cool");

	    i2c1.writeByte(ARDUINO_ADDR,0x30,00)

//	    i2c1.readByte(ARDUINO_ADDR,0x10)
//		.then((byte) => console.log(byte))
//		.then(() => {
//		    var outgoing = Buffer.from([1,2,3]);
//		    return(
//			i2c1.i2cWrite(ARDUINO_ADDR,3,outgoing)
//			    .then((result) => console.log("Write Result",result))
//		    );
//		})
//		.then(() => i2c1.writeByte(ARDUINO_ADDR,0x01,0xff))
	    
		.then(() => i2c1.close())
		.then(() => console.log("bus closed"))
		.catch((err) => console.error("Problem",err));

	} catch(err) {
	    console.error("Problem",err);
	}
    }

    firstTry();
}

***************/
