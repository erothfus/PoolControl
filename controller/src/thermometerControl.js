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
}

