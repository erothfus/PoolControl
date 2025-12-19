//
// heaterControl.js
//
//    Control the heater(s). But we really only have one.
//

module.exports = class {

    constructor(num)
    {
	this.heaterNum = num & 0x03;
    }

    status()
    {
	var command = 0xa0 | this.heaterNum;

	return(
	    Arduino.readBytes(command,4)
		.then((data) => {
		    return({enabled:data[0],
			    active:data[1],
			    setPoint:(data[2]<<8)|data[3],
			   });
		})
	);
    }

    config(tenths)
    {
	var command = 0xb0 | (0x02 << 2) | this.heaterNum;

	var sendArray =  [tenths>>8,tenths&0xff];
	return(
	    Arduino.writeBytes(command,sendArray.length,sendArray)
		.then(() => ({status:1}))
		.catch(() => ({status:0}))
	);
    }
    
    enable(onoff)
    {
	var command = 0xb0 | ((onoff & 0x01) << 2) | this.heaterNum;
	return(
	    Arduino.writeByte(command,0)
		.then(() => ({status:1}))
		.catch(() => ({status:0}))
	);
    }
}
	
