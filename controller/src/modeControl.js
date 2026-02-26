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

// VALVE 0 is the jets
const VALVE_0_SPA = 0;
const VALVE_0_POOL = 180;
const VALVE_0_BOTH = 90;

// VALVE 1 is the drains/returns
const VALVE_1_SPA = 0;
const VALVE_1_POOL = 180;
const VALVE_1_BOTH = 90;

// New Mode Mechanism
//
//  After talks with the client :-) I have determined that switching
//  modes needs to be a process, as opposed to simply establishing
//  state. For example, when moving between some modes, the pump
//  should be shut off prior to moving the valves.
//
//  I tried a fun plan set-up where there were both parallel and
//  sequential operations, but believe I was smacking into two
//  threads entering i2c at once, and results got scrambled. Sure,
//  I could go into the i2c library, but instead, I turn everything
//  into sequential. But this means that because of the above issue
//  (from the "client") I needed to create pseudo objects for the
//  valves, so that their completion becomes a "thing".
//
//  Note that this is an array (ordered) plan, where each element is
//  an "instruction".

var Mode_AllOff_Plan = [
    { heater: 0 },     // heater off right away
    { pump0: 0 },      // zero is off, one is low speed, two is high speed
    { pump1: 0 },      // zero is off, one is on
    { light: 0 },      // this mode actually turns EVERYTHING off
];

var Mode_SPA_Plan = [
    { heater: 0 },
    { pump0: 0 },
    { pump1: 0 },              // both pumps go off at first
    { valve0: VALVE_0_SPA },   // start the valves moving
    { valve1: VALVE_1_SPA },
    { valveWait: 0 },          // wait for each vale to stop moving
    { valveWait: 1 },
    { pump0: 2 },              // fire back up the main pump
    { delay: 3 },              // pump spin-up before turning on heater
    { heater: 1 },             // then turn on the heater
];

var Mode_SPAClean_Plan = [
    { heater: 0 },
    { pump0: 0 },
    { pump1: 0}, 
    { valve0: VALVE_0_POOL },
    { valve1: VALVE_1_BOTH },
    { valveWait: 0 },
    { valveWait: 1 },
    { pump0: 2 },
];

var Mode_HighFilter_Plan = [
    { heater: 0 },
    { pump0: 0 },
    { pump1: 0},                         // both pumps go off at first
    { valve0: VALVE_0_BOTH },
    { valve1: VALVE_1_POOL },  // move the valves together
    { valveWait: 0 },
    { valveWait: 1 },
    { pump0: 2 },
];

var Mode_LowFilter_Plan = [
    { heater: 0 },
    { pump0: 0 },
    { pump1: 0},                         // both pumps go off at first
    { valve0: VALVE_0_BOTH },
    { valve1: VALVE_1_POOL },  // move the valves together
    { valveWait: 0 },
    { valveWait: 1 },
    { pump0: 1 },
];

var Mode_WarmPool_Plan = [
    { heater: 0 },
    { pump0: 0 },
    { pump1: 0},
    { valve0: VALVE_0_BOTH },
    { valve1: VALVE_1_POOL },
    { valveWait: 0 },
    { valveWait: 1 },
    { pump0: 2 },
    { delay: 3 },            // pump spin-up before turning on heater
    { heater: 1 },
];

// here are the names for each of resources availble, along with their
//  mappings to objects that control them. These keys are those that MUST
//  be used in mode setting definitions.

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

// commands can be part of the plan, but aren't ever part of settings.

const commands = { 'valveWait':(valve)=>Valves[valve].wait(),
		   'delay':(secs)=> new Promise((res) => setTimeout(res,secs*1000))
		 };

// MODE NAMES - are defined here - you must use these to switch modes.
//   Each mode has the plan that implements the mode.

const availableModes = {
    "allOff"  :Mode_AllOff_Plan,
    "high"    :Mode_HighFilter_Plan,
    "low"     :Mode_LowFilter_Plan,
    "spa"     :Mode_SPA_Plan,
    "spaClean":Mode_SPAClean_Plan,
    "warmPool":Mode_WarmPool_Plan,
};


class ControlMode {

    myPlan;        // things going off/on in a particular order
    myName;        // because why not?
    mySettings;    // the results of executing myPlan

    constructor(name,plan)
    {
	this.myName = name;
	this.myPlan = plan;
	this.mySettings = this.computeSettings();
    }

    //
    // setMode() - given my plan, send it off to the hardware.
    //   Note that this SETS THE MODE - no matter what it currently
    //   looks like. Not optimized, but "for sure".
    //
    //   BIG NOTE - this FAILS if we're in the middle of setting
    //   the mode already. (how to do this???)
    //
    async setMode()
    {
	let chain = Promise.resolve(0);

	// A plan has an array of objects, each object has a resource
	//   setting or command. The plan should be traversed in order,
	//   chaining things such that one instruction occurs AFTER the
	//   other one is done.
	//
	// Note that this routine builds up the complete chain prior
	//   to returning.

	this.myPlan.forEach((instruction) => {
	    var resource = Object.keys(instruction)[0];   // all but #1 ignoerd

	    if(resources.hasOwnProperty(resource)) {     // command or resource setting?
		chain = chain.then(() => resources[resource].set(instruction[resource]));
	    } else {
		chain = chain.then(() => commands[resource](instruction[resource]));
	    }
	});

	return(chain);
    }

    //
    // computeSettings() - given my plan, compute the settings that
    //    will result after the plan is run - ignoring what happened
    //    while this mode was being set-up (because pumps are normally
    //    turned off before moving valves for example).
    //
    //    This is goes through the plan, merging the resource settings
    //    so that the last setting take precedence.
    //
    //    If there are resources that aren't mentioned in the plan,
    //    they are set to null, indicating that the plan didn't care
    //    about that resource.
    //
    computeSettings()
    {
	var settings = {};
	for(var resource in resources) {
	    settings[resource] = null;
	}
	for(var instruction of this.myPlan) {
	    settings = Object.assign(settings,instruction);
	}
	return(settings);
    }
    

    // matches() - returns true or false, depending upon wether
    //    this mode matches the given settings. Note that this
    //    takes into account the "null", ignoring settings with
    //    the null.
    //
    //    "settings" refers to an object with one resource and value
    //    for that resource. If the value is null, then the value for
    //    that resource is "don't care."
    //
    matches(settings)
    {
	for(let resource in resources) {
	    if(settings.hasOwnProperty(resource) &&
	       settings[resource] !== null &&
	       this.mySettings.hasOwnProperty(resource) &&
	       this.mySettings[resource] !== null) {
		if(settings[resource] != this.mySettings[resource]) {
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
	    this.collection[mode] = new ControlMode(mode,availableModes[mode]);
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
