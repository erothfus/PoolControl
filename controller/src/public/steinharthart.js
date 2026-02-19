//
// steinharthart.js
//
//    Used to calculate the coefficients for resistance & temperature
//    values of a thermocouple.
//

//
// calculateCoefficients() - given three resistance measurements at the
//     three given temps (in celsius). Returns an object with A B C.
//
function calculateCoefficients(
    tA,tB,tC,   // three temperature data points - all in celsius
    rA,rB,rC    // associated measured resistance values at temps
)
{
    // convert temps to Kelvin
    tA += 273.15;
    tB += 273.15;
    tC += 273.15;
    
    const l1 = Math.log(rA);
    const l2 = Math.log(rB);
    const l3 = Math.log(rC);
    const y1 = 1 / tA;
    const y2 = 1 / tB;
    const y3 = 1 / tC;
    const u2 = (y2 - y1) / (l2 - l1);
    const u3 = (y3 - y1) / (l3 - l1);
    const C = ((u3 - u2) / (l3 - l2)) * Math.pow(l1 + l2 + l3, -1);
    const B = u2 - C * (Math.pow(l1, 2) + l1 * l2 + Math.pow(l2, 2));
    const A = y1 - (B + Math.pow(l1, 2) * C) * l1;

    return { A, B, C };
}
