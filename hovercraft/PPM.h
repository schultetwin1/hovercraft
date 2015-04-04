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
  static volatile uint32_t pulseStart;
  static volatile uint32_t pulses[NUM_CHANNELS];
  
  static void isr();
};
#endif
