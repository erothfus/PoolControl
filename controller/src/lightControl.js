//
// lightControl.js
//
//    Just control the pool light relay. Like with everything
//    else, the code plans for multiple lights.
//

module.exports = class {
    
  constructor(num)
    {
	this.lightNum = num & 0x03;
    }

    status()
    {
	var command = 0xc0 | this.heaterNum;

	return(
	    Arduino.readBytes(command,1)
		.then((data) => {
		    return({status:data[0]});
		})
	);
    }

    control(onoff)
    {
	var command = 0xd0 | ((onoff & 0x01) << 2) | this.heaterNum;
	return(
	    Arduino.writeByte(command,0)
		.then(() => ({result:true}))
		.catch(() => ({result:false}))
	);
    }
}
