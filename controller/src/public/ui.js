//
// ui.js
//
//   All of the javascript necessary to support the ui is here.
//
//   This script needs to be "defer" so that it will run after the
//   rest of the HTML is loaded.
//

function degreeSlider(id,cls,min,max,value)
{
    // lay down a slider that will display its value, and call
    // the given function when the user stops interacting with it

    var newDiv = document.createElement('div');
    newDiv.classList.add('sliderDiv',cls);
    
    var newSlider = document.createElement('input');
    Object.assign(newSlider,{ type:'range', id, min, max, value, step:10 });
    newSlider.classList.add('slider');
    newDiv.appendChild(newSlider);

    var valDisplay = document.createElement('div');
    valDisplay.classList.add('sliderVal');
    newDiv.appendChild(valDisplay);

    newSlider.addEventListener('change', function() { console.log(this.value); });
    newSlider.addEventListener('input', function() { valDisplay.textContent = this.value; });

    return(newDiv);
}
