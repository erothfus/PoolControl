extern SHcoefficients(
    byte, byte, byte,				// first three are temperatures - 0 to 255 F
    unsigned int, unsigned int, unsigned int,	// the next are resistance values
    float*, float*, float*			// these are the three values returned
);
