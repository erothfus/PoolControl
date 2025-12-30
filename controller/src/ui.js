//
// ui.js
//
//   Implements all of the UI stuff - from presentation of pages to
//   the REST API for control.
//

const https = require('https');
const http = require('http');
const fs = require('fs');
const express = require('express');
const app = express();
const path = require('path');

// the general file-based interface must be installed
//   in the appropriate area - for now...
app.use(express.static(path.join(__dirname,'public')));

// all calls for "things to happen" come in to the "/api" tree
// the calls that are processed are below
//
//   /api - the base tree for the api
//   /api/valve - managing valves
//   /api/therm - managing thermometers
//   /api/pump - 
//   /api/heat -
//   /api/light -
//
//   VALVES
//      .../[#]/calibrate
//      .../[#]/status

// sub-routers are here

const valveAPI = express();  app.use('/api/valve',valveAPI);
const thermAPI = express();  app.use('/api/therm',thermAPI);
const pumpAPI = express();   app.use('/api/pump',pumpAPI);
const heaterAPI = express(); app.use('/api/heater',heaterAPI);
const lightAPI = express();  app.use('/api/light',lightAPI);
const modeAPI = express();   app.use('/api/mode',modeAPI);

modeAPI.get('/set/:mode',(req,res) => {
    ModeControl.setMode(req.params.mode)
	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
});

//
// /api/mode/check - returns the mode along with all of the settings
//        
modeAPI.get('/check',(req,res) => {
    ModeControl.findMode()
	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
});

lightAPI.get('/:light/status', (req,res) => {
    if(req.params.light >= Lights.length) {
	res.send(`ERROR - light ${req.params.light} unknown`);
    } else {
	Lights[req.params.light].status()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

lightAPI.get('/:light/control/:control', (req,res) => {
    if(req.params.light >= Lights.length) {
	res.send(`ERROR - light ${req.params.light} unknown`);
    } else {
	Lights[req.params.light].control(Number(req.params.control)?1:0)
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

heaterAPI.get('/:heater/config/:config', (req,res) => {
    if(req.params.heater >= Heaters.length) {
	res.send(`ERROR - heater ${req.params.heater} unknown`);
    } else {
	Heaters[req.params.heater].config(req.params.config)
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

heaterAPI.get('/:heater/status', (req,res) => {
    if(req.params.heater >= Heaters.length) {
	res.send(`ERROR - heater ${req.params.heater} unknown`);
    } else {
	Heaters[req.params.heater].status()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

heaterAPI.get('/:heater/enable/:enable', (req,res) => {
    if(req.params.heater >= Heaters.length) {
	res.send(`ERROR - heater ${req.params.heater} unknown`);
    } else {
	res.send(Heaters[req.params.heater].enable(Number(req.params.enable)?1:0));
    }
});

pumpAPI.get('/:pump/set/:speed', (req,res) => {
    if(req.params.pump >= Pumps.length) {
	res.send(`ERROR - valve ${req.params.pump} unknown`);
    } else {
	res.send(Pumps[req.params.pump].setSpeed(req.params.speed));
    }
});

pumpAPI.get('/:pump/status', (req,res) => {
    if(req.params.pump >= Pumps.length) {
	res.send(`ERROR - valve ${req.params.pump} unknown`);
    } else {
	Pumps[req.params.pump].status()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});
    

thermAPI.get('/:therm/read', (req,res) => {
    if(req.params.therm >= Thermometers.length) {
	res.send(`ERROR - valve ${req.params.therm} unknown`);
    } else {
	Thermometers[req.params.therm].read()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

valveAPI.get('/:valve/calibrate',(req,res) => {
    if(req.params.valve >= Valves.length) {
	res.send(`ERROR - valve ${req.params.valve} unknown`);
    } else {
	res.send(Valves[req.params.valve].calibrate());
    }
});

valveAPI.get('/:valve/status',(req,res) => {
    if(req.params.valve >= Valves.length) {
	res.send(`ERROR - valve ${req.params.valve} unknown`);
    } else {
	Valves[req.params.valve].status()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

valveAPI.get('/:valve/degrees',(req,res) => {
    if(req.params.valve >= Valves.length) {
	res.send(`ERROR - valve ${req.params.valve} unknown`);
    } else {
	Valves[req.params.valve].degrees()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

valveAPI.get('/:valve/travel',(req,res) => {
    if(req.params.valve >= Valves.length) {
	res.send(`ERROR - valve ${req.params.valve} unknown`);
    } else {
	Valves[req.params.valve].travelTimes()
//	    .then((data) => { console.log(data); return(data); })
	    .then((data) => JSON.stringify(data))
	    .then((json) => res.send(json));
    }
});

valveAPI.get('/:valve/move/:degrees',(req,res) => {
    if(req.params.valve >= Valves.length) {
	res.send(`ERROR - valve ${req.params.valve} unknown`);
    } else {
	res.send(Valves[req.params.valve].move(req.params.degrees));
    }
});
    

app.listen(8080,() => { console.log("web services up"); });

/***
    The following can be used to turn on https for this
    express server to do itself.
***/

/************	   
	   
const options = {
    key: fs.readFileSync('src/key.pem', 'utf8'),
    cert: fs.readFileSync('src/cert.pem', 'utf8'),
};

// Create the HTTPS server
https.createServer(options, app).listen(443, () => {
  console.log('HTTPS server listening on port 443');
});

// Redirect HTTP to HTTPS
http.createServer((req, res) => {
    res.writeHead(301, { "Location": "https://" + req.headers['host'] + req.url });
    res.end();
}).listen(80);

****************/
