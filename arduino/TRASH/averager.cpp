
// class members

  // reading the current flowing through the relay is being done continuously,
  //   averaging along the way. The following internal variables are used to
  //   make the whole thing work.  "ca" = "current average"

  int caReadBuffer[CA_SAMPLES];	// the buffer of current readings, time-oriented
  long caReadSum;		// the sum of the numbers in the buffer - cleverly maintained
  unsigned long caReadTime;	// keeps the timestamp for the last reading (micros)
  int caReadPtr;		// points to the previous read value in the buffer



  void readCurrentAverager(void);


//
// readCurrentAverager() - this routine is meant to be called by the
//    continuous loop, and will continually average the last
//    CA_SAMPLES of the current readings for the relay current. The
//    value is available at any time. Samples are collected at a
//    maximum of CA_PACING.
//
void Valve::readCurrentAverager(void)
{
  int current = analogRead(pin_monitor);// between 0 and 1023 inclusive

  unsigned long elapsed = CA_PACING_MS;	// defaults to normal pacing
  unsigned long now = micros();

  //  if(caReadPtr != -1) {		// the first time we just read
  //    if(now < caReadTime) {	// micros() roll-over
  //      elapsed = now + ((0UL - 1UL)-caReadTime);

    
  long long elapsed_us =
    (currentTime.tv_sec - caReadTime.tv_sec) * 1000000000LL +
    (currentTime.tv_nsec - caReadTime.tv_nsec);

  // only take readings every CA_PACING ns max - or the first time
  
  if(caReadPtr != -1 && elapsed_ns < CA_PACING) {
    return;
  }

  caReadTime = currentTime;
  
  if(caReadPtr == -1) {
    // the first time this is run, make the average the current value
    for(int i=0; i < CA_SAMPLES; i++) {	// fill buffer with this value
      caBuffer[i] = current;
    }
    caReadPtr = 0;
    caReadSum = current * CA_SAMPLES;
    caReadAvg = current;
  } else {
    long oldestReading = caReadBuffer[caReadPtr];
    caReadSum -= oldestReading - current;
    caReadAvg = caReadSum / CA_SAMPLES;
    
    caReadPtr++;
    if(caReadPtr == CA_SAMPLES) {
      caReadPtr = 0;
    }
    caReadBuffer[caReadPtr] = current;
  }
}
