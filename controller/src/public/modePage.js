//
// modePage.js
//
//   Implements the mode display and clicking. Only
//   used on the main index page.
//

//
// getMode() - gets the current mode on the controller. Returns a
//    Promise that resolves to an object:
//         { mode:<mode>, settings:<settings> }
//    Note that the mode may return "custom" - meaning that things
//    have changed beyond a particular mode.
//
function getMode()
{
    return(
	fetch(`/api/mode/check`)
	    .then((response)=> {
		if(!response.ok) {
		    throw new Error(`HTTP error. Status: ${response.status}`);
		} else {
		    return(response.json());
		}
	    })
    );
}

//
// modeDisplay() - light-up the appropriate mode button, or the "custom"
//    area otherwise.
//
function modeDisplay(mode)
{
    var selections = document.querySelectorAll('.controller .selection');

    for(var element of selections) {
	if([...element.classList].includes(mode)) {
	    element.classList.add('on');
	} else {
	    element.classList.remove('on');
	}
    }
}

//
// lightDisplay() - turns on/off the pretty light indicator.
//
function lightDisplay(onoff)
{
    var lightIcon = document.querySelector('#light-control .icon');
    var lit = lightIcon.querySelector('.image.lit');
    var unlit = lightIcon.querySelector('.image.unlit');

    function lightOFF() {
	lit.classList.remove('on');
	unlit.classList.add('on');
    }
    function lightON() {
	lit.classList.add('on');
	unlit.classList.remove('on');
    }

    (onoff)?lightON():lightOFF();
}

//
// wireUpButtons() - "wire-up" all of the buttons that can be pressed,
//   including the mode buttons and the light switch. Returns nothing
//   useful.
//
function wireUpButtons()
{
    // first, wire up the mode buttons

    function controlMode(mode)
    {
	return(
	    fetch(`/api/mode/set/${mode}`)
		.then((response)=> {
		    if(!response.ok) {
			throw new Error(`HTTP error. Status: ${response.status}`);
		    } else {
			return(response.json());
		    }
		})
	);
    }

    var selections = document.querySelectorAll('.controller .selection');
    selections.forEach((selection) => {
	if(![...selection.classList].includes('custom')) {
	    selection.addEventListener('click',() => {
		var mode = [...selection.classList].filter((item) => item != 'selection' && item != 'on')[0];
		controlMode(mode);
		modeDisplay(mode);
	    })
	}
    })

    // then wire up the light switch

    var lightControl = document.querySelector('#light-control');
    var lightIcon = lightControl.querySelector('.icon');
    var lit = lightIcon.querySelector('.image.lit');
    var unlit = lightIcon.querySelector('.image.unlit');

    function lightIS_ON() {
	return([...lit.classList].includes('on'));
    }

    function controlLight(onoff)
    {
	return(
	    fetch(`/api/light/0/control/${onoff}`)
		.then((response)=> {
		    if(!response.ok) {
			throw new Error(`HTTP error. Status: ${response.status}`);
		    } else {
			return(response.json());
		    }
		})
	);
    }

    lightControl.addEventListener('click',() => {
	// toggles the light
	if(lightIS_ON()) {
	    lightDisplay(0);
	    controlLight(0);
	} else {
	    lightDisplay(1);
	    controlLight(1);
	}
    });
}    

//
// updateScreen() - meant to be called upon start-up, and every now and then,
//     paint the mode screen appropriately. Doesn't return anything of note.
//
function updateScreen()
{
    getMode()
	.then((settings) => { console.log(settings); return(settings); })
	.then((settings) => modeSettings = settings)
	.then((settings) => { lightDisplay(settings.settings.light); return(settings); })
	.then((settings) => modeDisplay(settings.mode));
}    

//
// modeStartUp() - start-up the mode page. Finds the current mode and settings
//    and paints the screen appropriately. Also, wires up all of the buttons
//    to change modes as needed.
//
function modeStartUp()
{
    updateScreen();
    wireUpButtons();
}

modeStartUp()
{
}
