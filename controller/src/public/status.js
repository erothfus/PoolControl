//
// status.js
//
//    Reads the status of all system parameters and (re)paints them on the current
//    display.
//

function valveStatusGet(valve)
{
    return(
	fetch(`/api/valve/${valve}/status`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
//	    .then((data) => { console.log(data); return(data); })
    );
}

function valveTravelGet(valve)
{
    return(
	fetch(`/api/valve/${valve}/travel`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
//	    .then((data) => { console.log(data); return(data); })
    );
}
    

function valveStatusDisplay(valve,data)
{
    var target = document.querySelector(`.valve.v${valve} .degrees`);
    target.textContent = data.position;

    target = document.querySelector(`#v${valve}-slider`);
    target.value = data.position;
}

//
// statusGet() - this is the settings display updater. It runs periodically
//   to update the data on the settings dispay - as well as the dialogs.
//   Note that it effectively stops updating the dialogs if they are
//   displayed.
//
function statusGet()
{
    var delay = 5000;          // 5 seconds between updates
    
    //
    // isDialogDisplayed() - throws an error if a dialog is displayed,
    //    causing the display NOT to occur, but the loop continues.
    //
    async function noDialogDisplayed()
    {
	var v0Dialog = document.getElementById('settings-v0');
	var v1Dialog = document.getElementById('settings-v1');
	var tempDialog = document.getElementById('settings-temp');

	if(v0Dialog.open || v1Dialog.open || tempDialog.open) {
	    throw "Dialog open"
	}
    }
	
    return(
	noDialogDisplayed()

	// these are only run if there is no dialog displayed
	    .then(() =>	valveStatusGet(0))
	    .then((data) => valveStatusDisplay(0,data))
	    .then(() => valveStatusGet(1))
	    .then((data) => valveStatusDisplay(1,data))
	    .then(() => pumpStatusGet(0))
	    .then((data) => pumpStatusDisplay(0,data))
	    .then(() => pumpStatusGet(1))
	    .then((data) => pumpStatusDisplay(1,data))
	    .then(() => thermStatusGet(0))
	    .then((data) => thermStatusDisplay(0,data))
	    .then(() => heaterStatusGet(0))
	    .then((data) => heaterStatusDisplay(0,data))
	    .then(() => lightStatusGet(0))
	    .then((data) => lightStatusDisplay(0,data))

	// catch the case where there is a dialog displayed
	    .catch(() => {})

	// the setTimeout happens in any case
	    .then(() => setTimeout(statusGet,delay))
    );
}

function lightStatusGet(light)
{
    // get the status of the light - which is an object with
    //   { status:|0,1| }

    return(
	fetch(`/api/light/${light}/status`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}

function lightStatusDisplay(light,data)
{
    var led = data.status;
    ledTurnOn(".light",led);
}

function lightSet(light,onoff)
{
    return(
	fetch(`/api/light/${light}/control/${onoff}`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}

function heaterStatusGet(heater)
{
    // get the status of the heater - which is an object with
    //   { enabled, active }

    return(
	fetch(`/api/heater/${heater}/status`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}

function heaterStatusDisplay(heater,data)
{
    // incoming data has {enabled:[0,1],active:[0,1]} - which refers to
    //   whether the heater is enabled and current active (or not)
    var led = data.enabled;
    ledTurnOn(`.heater`,led);
    ledTurnOn('.heater','-flame',data.active);

    // go ahead and update the setting dialog
    var target = document.querySelector("#settings-temp .tempDisplay");
    target.value = Math.floor(data.setPoint/10);

    target = document.querySelector("#settings-temp .slider input");
    target.value = Math.floor(data.setPoint/10);
}

function heaterSet(heater,enabled)
{
    return(
	fetch(`/api/heater/${heater}/enable/${enabled}`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}

//
// heaterSetSetPoint() - given a heater number (only one for now) and a temp
//    in tenths of degrees, set the heater set-point.
//
function heaterSetSetPoint(heater,setPoint)
{
    return(
	fetch(`/api/heater/${heater}/config/${setPoint}`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}

function thermStatusGet(therm)
{
    // get the thermometer reading

    return(
	fetch(`/api/therm/${therm}/read`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
//	    .then((data) => { console.log(data); return(data); })
    );
}

function thermStatusDisplay(therm,data)
{
    var target = document.querySelector(`.thermometer.thermometer${therm} .degrees`);
    target.textContent = Math.floor(data.temp/10);
//    console.log("setting",data.position);
}

function pumpSet(pump,speed)
{
    // set the speed for the given pump - assumes 0 <= speed <= 2
    //   and 2 doesn't happen for the booster

//    console.log(`pumpSet() - /api/pump/${pump}/set/${speed}`);
    
    return(
	fetch(`/api/pump/${pump}/set/${speed}`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
    );
}
	
function pumpStatusGet(pump)
{
    // get the status here for the pump - which will return 0, 1, or 2 for off, on, high

    return(
	fetch(`/api/pump/${pump}/status`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
//	    .then((data) => { console.log(data); return(data); })
    );
}

function pumpStatusDisplay(pump,data)
{
    // incoming data has {status:[0,1,2]} - which refers to
    //   the particular LED that should turn on

    console.log("PUMP STATUS",pump,data);
    var led = data.status;
    ledTurnOn(`.pump.p${pump}`,led);
}

//
// ledCallbacks() - set-up the callbacks for clicking on an led.
//     But pay attention to those leds (or led-like things)
//     that don't want to be clicked.
//
function ledCallbacks()
{
    // set-up pump led callbacks - just hardwired for ease

    for(let pump of [0,1]) {
	document.querySelectorAll(`.entry.pump.p${pump} .led`).forEach((led) => {
	    var ledID = [...led.classList].filter((c) => c!= 'led')[0][3];
	    led.querySelector('.imageSwitchable')
		.addEventListener('click',() => pumpSet(pump,ledID))
	});
    }

    // heater callbacks

    document.querySelectorAll(`.entry.heater .led`)
	.forEach((led) => {
	    let ledID = ledName(led.classList);
	    if(ledID != '-flame') {
		led.querySelector('.imageSwitchable')
		    .addEventListener('click',() => heaterSet(0,ledID));
	    }
	});

    // light callbacks
    
    document.querySelectorAll(`.entry.light .led`)
	.forEach((led) => {
	    let ledID = ledName(led.classList);
	    led.querySelector('.imageSwitchable')
		.addEventListener('click',() => lightSet(0,ledID));
	});

    // this code adds the exclusivity to the buttons - when one is clicked
    //   all of the led images adjust appropriately - if appropriate
    
    document.querySelectorAll('.status .entry')
	.forEach((entry) => {
	    var entryLEDs = entry.querySelectorAll('.led');
	    if(entryLEDs.length == 0) {
		return;
	    }
	    let entryName = '.' + [...entry.classList].filter((e) => e != 'entry').join('.');

	    entryLEDs.forEach((led) => {
		var clickable = !([...led.classList].includes('non-clickable'));
		if(clickable) {
		    let ledID = ledName(led.classList);
		    led.querySelector('.imageSwitchable')
			.addEventListener('click',() => ledTurnOn(entryName,ledID));
		}
	    })
	});
}

//
// ledName() - a subroutine to make getting the name of an led from
//    the classlist easier - and to filter out the other options
//    You pass the classlist directly from the javascript selector,
//    you DON'T have to "..." it first.
//
function ledName(classlist)
{
    // we assume that the class list has "led", and maybe one of the
    //   "non-" options, and then an "ledXXXX" where XXXX is the id
    //   of the led.
    
    var name = [...classlist]
	.filter((c) => c != 'led' && c != 'non-exlusive' && c != 'non-clickable')[0];

    // skip by the leading "led" and return the rest of the name
    return(name.substring(3));
}

//
// ledTurnOn() - turn ON a particular LED while turning off the others.
//    The leds go from 0 to 2. The entry must be an appropriate selector
//    for the given entry that has LEDs - for example ".pump.p0"
//
//    This has been modified to allow for "non-clickable" and "non-exclusive"
//    leds. Although, the "led" may be any two pictures for on/off.
//
//    BIG NOTE - for those leds that are non-exclusive, the "onoff" is used
//    to indicate whether they should be turned on (truey) or off (falsey).
//
function ledTurnOn(entrySelector,led,onoff)
{
    // check the classes on the selected element for exclusivity (radio-buttons)

    var classlist = document.querySelector(`.entry${entrySelector} .led${led}`).classList;
    var exclusive = !([...classlist].includes('non-exclusive'));
    
    if(exclusive) {    // first turn off ALL led png's if exclusive
	document.querySelectorAll(`.entry${entrySelector} .led .image`)
	    .forEach((el) => el.classList.remove('on'));

	// then turn on all of the unlit ones
	document.querySelectorAll(`${entrySelector} .led .image.unlit`)
	    .forEach((el) => el.classList.add('on'));
    }

    if(exclusive || onoff) {
	// now turn on the target lit on while turning off the unlit
	document.querySelector(`${entrySelector} .led${led} .image.lit`).classList.add('on');
	document.querySelector(`${entrySelector} .led${led} .image.unlit`).classList.remove('on');
    } else {
	// turn OFF the non-exclusive target
	document.querySelector(`${entrySelector} .led${led} .image.lit`).classList.remove('on');
	document.querySelector(`${entrySelector} .led${led} .image.unlit`).classList.add('on');
    }
}

// pop-ups are used for status settings. But they need to programmed
//   to operate appropriately both internally/display and regarding
//   the status data
//
// there are movable pieces to the status pop-ups

//
// tempSetPoint() - program the temp setpoint controls to all work together

function tempSetPoint()
{
    var max = 110;
    var min = 50;
    
    document.querySelectorAll('.tempSetPoint')
	.forEach((elem) => {
	    let slider = elem.querySelector('.slider input');  // slider element
	    let number = elem.querySelector('.tempDisplay');   // text input element
	    let up = elem.querySelector('.updowns .up');       // up button
	    let down = elem.querySelector('.updowns .down');   // down button

	    // when the slider changes, update the number
	    //   and set the value
	    slider.addEventListener('change',(event) => {
		number.value = slider.value;
		heaterSetSetPoint(0,number.value*10);
	    })

	    // BIG NOTE - this code assumes that the input field is set as "readonly"
	    //    because it is problematic having the temp be input directly, because
	    //    "when do you update?"
	    //
	    //number.addEventListener('change',(event) => {
	    //	if(Number(number.value) > max) {
	    //	    number.value = max;
	    //	}
	    //	if(Number(number.value) < min) {
	    //	    number.value = min;
	    //	}
	    //	slider.value = number.value;
	    //	console.log("number",number.value);
	    //})
	    
	    up.addEventListener('click',(event) => {
		if(slider.value < max) {
		    slider.value = Number(slider.value) + 1;
		    number.value = Number(number.value) + 1;
		    heaterSetSetPoint(0,number.value*10);
		}
	    })
	    down.addEventListener('click',(event) => {
		if(slider.value > min) {
		    slider.value = Number(slider.value) - 1;
		    number.value = Number(number.value) - 1;
		    heaterSetSetPoint(0,number.value*10);
		}
	    })
	});
}

//
// valveDisplaySliderPosition() - show the given position on the given
//     valve slider.
//
function valveDisplaySliderPosition(valve,value)
{
    var slider = document.querySelector(`#v${valve}-slider`);
    if(slider) {
	slider.value = value;
    }
}
    

//
// valveSetPosition() - used to put the valve in a known position. The
//    code is currently locked at 0 to 180, although it would be nice
//    to have that settable.
//
function valveSetPosition(valve)
{
    // get the status of the valve so that the position can be 
    // set-up the slider to control the particular valve

    var slider = document.querySelector(`#v${valve}-slider`);
    if(slider) {
	slider.addEventListener('change',(event) => valveMove(valve,slider.value))
    }

    // when the dialog is up, the position status of the 
}

//
// valveDialogPopup() - called to pop-up the given valve's dialog for
//    control/config.
//
function valveDialogPopup(valve)
{
    // show the dialog right away
    var dialog = document.getElementById(`settings-v${valve}`);
    dialog.showModal();

    // this sub-function updates the controls with the current valve
    //   status data. It schedules itself to be called if the dialog
    //   is still displayed.
    function updateDisplay()
    {
	valveStatusGet(valve)
	    .then((status) => {
		valveDisplaySliderPosition(valve,status.position);
		valveDisplayStateMachineCodes(valve,status);
	    })

	    .then(() => valveTravelGet(valve))
	    .then((travel) => valveDisplayValveTravel(valve,travel))

	    .then(() => {
		if(dialog.open) {
		    setTimeout(updateDisplay,3000);
		}
	    });
    }

    updateDisplay();
}

//
// valveDisplayStateMachineCodes() - display the status codes on the popup display.
//
function valveDisplayStateMachineCodes(valve,status)
{
    var dialog = document.getElementById(`settings-v${valve}`);
    
    var currentElement = dialog.querySelector('.valveStateDisplay .current');
    currentElement.querySelector('.code').textContent = status.state;
    currentElement.querySelector('.descr').textContent = valveStateMachine[status.state];

    var prevElement = dialog.querySelector('.valveStateDisplay .prev');
    prevElement.querySelector('.code').textContent = status.prev;
    prevElement.querySelector('.descr').textContent = valveStateMachine[status.prev];

    
    console.log("Status",status);
}

//
// valveDisplayValveTravel() - given a valve and travel objecct, display it on the
//   status display in the "appropriate" place.
//
function valveDisplayValveTravel(valve,travel)
{
    var dialog = document.getElementById(`settings-v${valve}`);

    var element;

    element = dialog.querySelector('.calibrationStatus .clockwise');
    element.querySelector('.value').textContent = (Number(travel.pos)/10).toFixed(2);

    element = dialog.querySelector('.calibrationStatus .counterclockwise');
    element.querySelector('.value').textContent = (Number(travel.neg)/10).toFixed(2);
}

function valveInitiateCalibrationRequest(valve)
{
    if(confirm("Are you sure you want to calibrate?")) {
	valveInitiateCalibration(valve);
    }
}

function valveMove(valve,value)
{
    return(
	fetch(`/api/valve/${valve}/move/${value}`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
	    .then((status) => valveDisplayStateMachineCodes(valve,status))
//	    .then((data) => { console.log(data); return(data); })
    );
}

function valveInitiateCalibration(valve)
{
    return(
	fetch(`/api/valve/${valve}/calibrate`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
//	    .then((data) => { console.log(data); return(data); })
    );
}

// Termometer configuration pop-up routines

var scaleTemps = {
    F : { low:  34, medium: 68, high: 104 },
    C : { low: 1.1, medium: 20, high:  40 }
};
	

//
// thermConfigHandling() - handle the input fields and confirmation button
//    for the thermometer settings. Needs to be called upon init.
//
function thermConfigHandling()
{
    var calibrateButton = document.querySelector("#settings-therm .calibrate .button");
    var inputTemps = document.querySelector("#settings-therm .inputTemps");
    var lowTempInput = inputTemps.querySelector('.tempLow');
    var mediumTempInput = inputTemps.querySelector('.tempMedium');
    var highTempInput = inputTemps.querySelector('.tempHigh');

    var coefficients = document.querySelector("#settings-therm .coefficients");
    var cA = coefficients.querySelector('.value.A');
    var cB = coefficients.querySelector('.value.B');
    var cC = coefficients.querySelector('.value.C');

    // put listeners on the three input fields - which will enable/disable
    //   the submit button, as well as show the coefficients (for fun)
    
    for(var tempInput of [lowTempInput,mediumTempInput,highTempInput]) {
	tempInput.addEventListener('input',(e) => {
	    var full = lowTempInput.value != '' && mediumTempInput.value != '' && highTempInput.value != '';
	    enableSubmit(full);
	    if(full) {
		var coef = calculateCoefficients(scaleTemps["C"]["low"],
						 scaleTemps["C"]["medium"],
						 scaleTemps["C"]["high"],
						 lowTempInput.value,
						 mediumTempInput.value,
						 highTempInput.value);
		cA.textContent = coef.A.toExponential(6);
		cB.textContent = coef.B.toExponential(6);
		cC.textContent = coef.C.toExponential(6);
	    } else {
		cA.innerHTML = cB.innerHTML = cC.innerHTML = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	    }
	    
	})
    }

    // add a listener to the submit button

    calibrateButton.addEventListener('click', (e) => {
	var call = 'api/therm/0/config';
	call += `/${scaleTemps["F"]["low"]}`;
	call += `/${scaleTemps["F"]["medium"]}`;
	call += `/${scaleTemps["F"]["high"]}`;
	call += `/${lowTempInput.value}`;
	call += `/${mediumTempInput.value}`;
	call += `/${highTempInput.value}`;

	fetch(call)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		}
		return response.json();
	    })
	    .then((data) => { console.log("Therm config",data); return(data); })
    });
    
    setScale("F");   // the default set by the html
    enableSubmit(false);

    //
    // enableSubmit() - dis/enables the submit button for the thermometer
    //     config given the arg.
    //
    function enableSubmit(onoff)
    {
	if(onoff) {
	    calibrateButton.classList.remove('disabled');
	} else {
	    calibrateButton.classList.add('disabled');
	}
    }

}

//
// setScale() - sets either Fahrenheit or Celcius depending upon the arg.
//
function setScale(scale)
{
    var inputTemps = document.querySelector("#settings-therm .inputTemps");

    for(var temp of ["low","medium","high"]) {
	inputTemps.querySelector(`.${temp}`).textContent = scaleTemps[scale][temp];
    }
}


// INITIAL CONFIGURATION HERE

valveSetPosition(0);
valveSetPosition(1);

tempSetPoint();

ledCallbacks();

statusGet();

thermConfigHandling();
