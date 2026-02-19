//
// thermometerControl.js
//
//   Code for managing thermometers.
//

module.exports = class {

    constructor(num)
    {
	this.thermNum = num & 0x03;
    }

    read()
    {
	var command = 0x80 | this.thermNum;

	return(
	    Arduino.readBytes(command,2)
		.then((data) => {
//		    console.log("therm",data);
		    return({temp:(data[0]<<8)+data[1]});
		})
	);
    }

    //
    // config() - a little different from other routines, this one gets the
    //   REST params object directly so it can be called easily. That
    //   object as tA, tB, tC, rA, rB, rC in it.
    //
    config(params)
    {
	var command = 0b10010000 | this.heaterNum;

	var sendArray =  [
	    params.tA, params.tB, params.tC,
	    params.rA >> 8, params.rA&0xff,
	    params.rB >> 8, params.rB&0xff,
	    params.rC >> 8, params.rC&0xff
	];

	return(
	    Arduino.writeBytes(command,sendArray.length,sendArray)
		.then(() => ({status:1}))
		.catch(() => ({status:0}))
	);
    }
	
}

