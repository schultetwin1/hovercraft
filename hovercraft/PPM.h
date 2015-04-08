#ifndef __AERO205_PPM__
#define __AERO205_PPM__
#include "Arduino.h"

class PPM {
 public:
  static void begin();
  static void end();
  static uint32_t channelPulse(uint8_t channel);
  static bool controllerConnected();
  static void zeroPulses();
  
  static const int NUM_CHANNELS = 9;
  
 private:
  static volatile uint16_t curPulse;
  static volatile unsigned long pulseStart;
  static volatile uint32_t pulses[NUM_CHANNELS];
  
  // 3.5 ms minimum start pulse length
  static const int MIN_START_PULSE_LEN = 3500;
  // 10 ms timeout
  static const int PULSE_LEN_TIMEOUT = 10000;
  
  static void isr();
};
#endif
