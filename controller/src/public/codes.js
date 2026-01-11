//
// codes.js
//
//   Contains the special "codes" that are used within the Arduino code
//   so that a sophisticated user can see what is going on, particularly
//   with the state machines.
//

const valveStateMachine = {

    0:"Valve Inactive",
    1:"Seek Fail",
    
    100:"Cal. Start",
    120:"Cal. Start - 3 seconds negative",
    121:"Cal. Start - 3 seconds positive",
    122:"Cal. Start - quiescent",

    101:"Cal. Known State (neg) Seek",
    102:"Cal. Known State (neg) Initiation - spin-up",
    103:"Cal. Looking for Known State (neg) Limit",
    104:"Cal. Looking for Known State (neg) Limit - settling",
    105:"Cal. Found Known State (neg) Limit",

    106:"Cal. Positive Benchmark",
    107:"Cal. Positive Initiation - spin-up",
    108:"Cal. Looking for Positive Limit",
    109:"Cal. Looking for Positive Limit - settling",
    110:"Cal. Found Positive Limit",

    111:"Cal. Negative Benchmark",
    112:"Cal. Negative Initiation - spin-up",
    113:"Cal. Looking for Negative Limit",
    114:"Cal. Looking for Negative Limit - settling",
    115:"Cal. Found Negative Limit",

    200:"Move Low Limit Initiate",
    201:"Move Low Limit Current Wait",
    202:"Move Low Limit Current check",
    203:"Move Low Limit Done",

    230:"Move High Limit Initiate",
    231:"Move High Limit Current Wait",
    232:"Move High Limit Current check",
    233:"Move High Limit Done",

    210:"Move to Position Initiate (timed)",
    211:"Move to Position Done",
};

