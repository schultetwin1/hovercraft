#include "PPM.h"

const int RECVEIVER_PIN    =  2; // !! MUST BE ON PIN 2. Using hardware interrupt !!

volatile uint16_t PPM::curPulse = 0;
volatile uint32_t PPM::pulseStart = 0;
volatile uint32_t PPM::pulses[PPM::NUM_CHANNELS];

void PPM::isr(){
  uint16_t now = micros();
  // Handle PPM pin changes
   if(digitalRead(RECVEIVER_PIN) == HIGH){
     //rising edge
     PPM::pulseStart = now;
   } else {
     //falling edge
     uint16_t time = now - PPM::pulseStart;
     if(time > PPM::MIN_START_PULSE_LEN){
       // Greater than 3.5ms, therefore it's the start pulse.
       PPM::curPulse = 0;
       return;
     }
     if(PPM::curPulse >= PPM::NUM_CHANNELS){
       // we have a problem, controller disconnected?
       // continue current state
       PPM::zeroPulses();
       return; 
     }
     // Assign the pulse and increment the count.
     PPM::pulses[PPM::curPulse++] = time;
   }
}

void PPM::begin() {
  curPulse = 0;
  pulseStart = 0;
  
  zeroPulses();
  pinMode(RECVEIVER_PIN, INPUT);
  attachInterrupt(0,PPM::isr,CHANGE);
}

void PPM::end() {
  detachInterrupt(0);
  curPulse = 0;
  pulseStart = 0;
}

uint32_t PPM::channelPulse(uint8_t channel) {
  if (channel >= PPM::NUM_CHANNELS) return -1;
  
  return pulses[channel];
}

bool PPM::controllerConnected() {
  return (micros() - pulseStart) < PPM::PULSE_LEN_TIMEOUT;
}

void PPM::zeroPulses() {
  for (int i = 0; i < PPM::NUM_CHANNELS; i++) {
    pulses[i] = 0;
  }
}
