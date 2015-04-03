#include "PPM.h"

const int RECVEIVER_PIN    =  2; // !! MUST BE ON PIN 2. Using hardware interrupt !!

volatile uint16_t PPM::curPulse = 0;
volatile uint32_t PPM::pulseStart = 0;
volatile uint32_t PPM::pulses[NUM_PPM_CHANNELS];

void ISRPIN2(){
  // Handle PPM pin changes
   if(digitalRead(RECVEIVER_PIN) == HIGH){
     //rising edge
     PPM::pulseStart = micros();
   }else{
     //falling edge
     uint16_t time = micros() - PPM::pulseStart;
     if(time > 5000){
       // Greater than 5ms, therefore it's the gap.
       PPM::curPulse = 0;
       return;
     }
     if(PPM::curPulse >= NUM_PPM_CHANNELS){
       // we have a problem, controller disconnected?
       // continue current state
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
  attachInterrupt(0,ISRPIN2,CHANGE);
}

void PPM::end() {
  detachInterrupt(0);
  curPulse = 0;
  pulseStart = 0;
}

uint32_t PPM::channelPulse(uint8_t channel) {
  if (channel >= NUM_PPM_CHANNELS) return -1;
  
  return pulses[channel];
}

bool PPM::controllerConnected() {
  if (micros() - pulseStart > 30000) {
    return false;
  }
  return true;
}

void PPM::zeroPulses() {
  for (int i = 0; i < NUM_PPM_CHANNELS; i++) {
    pulses[i] = 0;
  }
}
