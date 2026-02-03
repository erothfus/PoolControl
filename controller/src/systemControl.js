//
// systemControl.js
//
//   "System" control functions, like factory reset.
//

module.exports = class {

    floopy = "hello";
    
    constructor()
    {
    }
    
    async factoryReset()
    {
	var command = 0xf0;    // command 0b111 + write bit
	return(
	    Arduino.writeByte(command,0)
		.then(() => ({result:true}))
		.catch(() => ({result:false}))
	);
    }
}
	    
