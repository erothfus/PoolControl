//
// steinharthart.cpp
//
//    Given 6 parameters, calculate the Steinhart-Hart coefficients
//    for reading the termistor temperature sensor.
//
//    Fills in the given 3 floats with the coefficients.
//
//    The temperatures are given as ints celsius, and are between 0 and 255
//

#include <Arduino.h>
#include <math.h>

float FtoKelvin(int fdeg)
{
    return(((float)fdeg - 32.0f) * 5.0f / 9.0f + 273.15f);
}

void SHcoefficients(
		    byte in_tA, byte in_tB, byte in_tC,
		    unsigned int in_rA, unsigned int in_rB, unsigned int in_rC,
		    float *A, float *B, float *C
		    )
{
    // all math is done in float land

    float tA, tB, tC;
    float rA, rB, rC;
    float L1, L2, L3;
    float Y1, Y2, Y3;
    float U2, U3;

    tA = FtoKelvin(in_tA);
    tB = FtoKelvin(in_tB);
    tC = FtoKelvin(in_tC);
    
    L1 = log((float)in_rA);
    L2 = log((float)in_rB);
    L3 = log((float)in_rC);
    
    Y1 = 1.0f / tA;
    Y2 = 1.0f / tB;
    Y3 = 1.0f / tC;

    U2 = (Y2 - Y1) / (L2 - L1);
    U3 = (Y3 - Y1) / (L3 - L1);

    *C = ((U3 - U2) / (L3 - L2)) * (1.0f / (L1 + L2 + L3));
    *B = U2 - *C * ((L1*L1) + L1 * L2 + (L2*L2));
    *A = Y1 - (*B + (L1*L1) * *C) * L1;
}
