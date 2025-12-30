//
// modeControl.js
//
//   The Pool Controller normally operates in a given "mode", which is
//   a collection of settings that change with the mode.
//   This file defines the modes, and their control mechanisms.
//
//   There are two interesting classes: ControlMode and ControlModeCollection.
//   The latter is the one that is returned. The caller interacts with the
//   collection and the ControlMode is used to define and then control the
//   actual modes.
//

// MODES - here is where all of the control modes are defined
//    They are used to create the objects that control them,
//    and become part of the ControlModeCollection.
//
// Each mode sets up to 6 things: heater, 2 pumps, 2 valves, and the
// light.  Note that the light is normally not changed, only when
// turning EVERYTHING off.

// VALVE 0 is the jets
const VALVE_0_SPA = 0;
const VALVE_0_POOL = 180;
const VALVE_0_BOTH = 90;

// VALVE 1 is the drains/returns
const VALVE_1_SPA = 0;
const VALVE_1_POOL = 180;
const VALVE_1_BOTH = 90;

// here are the names for each of resources availble, along with their
//  mappings to objects that control them. These keys are those that MUST
//  bust be used in mode setting definitions.

const resources = { 'heater':{set:(h) => Heaters[0].enable(h),
			      get:(h) => Heaters[0].status().then((status) => status.enabled)},
		    'valve0':{set:(v) => Valves[0].move(v),
			      get:(v) => Valves[0].status().then((status) => status.position)},
		    'valve1':{set:(v) => Valves[1].move(v),
			      get:(v) => Valves[1].status().then((status) => status.position)},
		    'pump0':{set:(p) => Pumps[0].setSpeed(p),
			     get:(p) => Pumps[0].status().then((status) => status.status) },
		    'pump1':{set:(p) => Pumps[1].setSpeed(p),
			     get:(p) => Pumps[1].status().then((status) => status.status) },
		    'light':{set:(l) => Lights[0].control(l),
			     get:(l) => Lights[0].status().then((status) => status.status) },
		  };

// NOTE - each mode MUST have a heater, pump0, pump1, valve0, valve1, light.

var Mode_AllOff = {
    heater: 0,     // zero is off, one is on
    pump0: 0,      // zero is off, one is low speed, two is high speed
    pump1: 0,      // zero is off, one is on
    valve0: null,  // "null" is a no-change/ignore value
    valve1: null,
    light: 0,      // this mode actually turns EVERYTHING off
};

var Mode_SPA = {
    heater: 1,
    pump0: 2,
    pump1: 0,
    valve0: VALVE_0_SPA,
    valve1: VALVE_1_SPA,
    light: null     // no change on light
};
    
var Mode_SPAClean = {
    heater: 0,
    pump0: 2,
    pump1: 0,
    valve0: VALVE_0_POOL,
    valve1: VALVE_1_BOTH,
    light: null     // no change on light
};
    
var Mode_HighFilter = {
    heater: 0,
    pump0: 2,
    pump1: 0,
    valve0: VALVE_0_BOTH,
    valve1: VALVE_1_POOL,
    light: null     // no change on light
};
    
var Mode_LowFilter = {
    heater: 0,
    pump0: 1,
    pump1: 0,
    valve0: VALVE_0_BOTH,
    valve1: VALVE_1_POOL,
    light: null     // no change on light
};
    
var Mode_WarmPool = {
    heater: 1,
    pump0: 2,
    pump1: 0,
    valve0: VALVE_0_BOTH,
    valve1: VALVE_1_POOL,
    light: null     // no change on light
};

// MODE NAMES - are defined here - you must use these to switch modes

const availableModes = {
    "allOff"  :Mode_AllOff,
    "high"    :Mode_HighFilter,
    "low"     :Mode_LowFilter,
    "spa"     :Mode_SPA,
    "spaClean":Mode_SPAClean,
    "warmPool":Mode_WarmPool,
};


class ControlMode {

    settings;    // the hardware settings for this mode

    constructor(mySettings)
    {
	this.settings = mySettings;
    }

    //
    // setMode() - given my settings, send it off to the hardware.
    //   Note that this SETS THE MODE - no matter what it currently
    //   looks like. Not optimized, but "for sure".
    //
    async setMode()
    {
	let chain = Promise.resolve(0);

	for(let setting in resources) {
	    if(this.settings[setting] !== null) {  // null settings are ignored
		console.log("setting ",setting," to ",this.settings[setting]);
		chain = chain.then(() => resources[setting].set(this.settings[setting]));
	    }
	}
	return(chain);
    }

    // matches() - returns true or false, depending upon wether
    //    this mode matches the given settings. Note that this
    //    takes into account the "null", ignoring settings with
    //    the null.
    //
    matches(settings)
    {
	for(let setting in resources) {
	    if(this.settings[setting] !== null) {
		if(settings[setting] != this.settings[setting]) {
		    return(false);
		}
	    }
	}
	return(true);
    }
    
}

class ControlModeCollection {

    collection = {};           // this is an array of ControlModes, indexed
                               //   by the mode name
    constructor()
    {
	for(var mode in availableModes) {
	    this.collection[mode] = new ControlMode(availableModes[mode]);
	}
    }

    async setMode(mode)
    {
	if(!this.collection.hasOwnProperty(mode)) {
	    return(false);
	}

	return(this.collection[mode].setMode());
    }

    //
    // getSettings() - run through the resources collecting the status
    //    of things, returning an object with the resource settings
    //    in it.
    //
    async getSettings()
    {
	var settings = {};
	var chain = Promise.resolve(0);
	
	for(let resource in resources) {
	    chain = chain
		.then(() => resources[resource].get())
		.then((status) => settings[resource] = status);
	}
	
	return(chain.then(() => settings));
    }

    //
    // findMode() - gets the current status of all resources, and returns
    //   the mode that matches those settings or "custom" if there are
    //   no matches.
    //
    //   Returns an object {mode:<mode>,settings:<settings>}
    //
    async findMode()
    {
	var modeSettings;
	
	return(
	    this.getSettings()
		.then((settings) => {
		    modeSettings = settings;
		    for(var mode in this.collection) {
			if(this.collection[mode].matches(settings)) {
			    return(mode);
			}
		    }
		    return("custom");
		})
		.then((mode) => ({mode:mode,settings:modeSettings}))
	);
    }			
    
}

module.exports = new ControlModeCollection();
