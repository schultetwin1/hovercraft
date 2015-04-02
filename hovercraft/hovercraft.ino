#include <Servo.h>

// These pin numbers have been choose carefully
// Please ask Matt before changing
// DO NOT USE analogWrite on pins 9 or 10, it will not work
// See (http://arduino.cc/en/reference/servo) for explination

// Pin 3 and Pin 11 use Timer 2
const int LIFT_MOTOR_PIN = 3;
const int THRUST_MOTOR_PIN = 11;
const int THRUST_DIR_PIN = 12;
const int PPM_PIN = 2; // !! MUST BE ON PIN 2 !! 

const int MOTOR_SERVO_PIN = 6;

const int LED_PIN = 13;

// Lift Input will be between 1000us and 1850us
// Output between 0 - 200 (not 255 to save the motor)
int MAX_LIFT_OUTPUT = 200;
const int MIN_LIFT_OUTPUT = 0;
const int MAX_LIFT_INPUT  = 1160;
const int MIN_LIFT_INPUT  = 572;

// Thrust Input will be between 1550us (halfway) and 1900us
// Output between 0 - 255
const int MAX_THRUST_OUTPUT = 255;
const int MIN_THRUST_OUTPUT = 0;
const int MAX_THRUST_INPUT  = 1584;
const int MIN_THRUST_INPUT  = 760;
const int MID_THRUST_INPUT  = 1180;

// Lift Input will be between 1060 and 1860us
// Output between 900 - 2100 us
const int MAX_MOTOR_DIR_OUTPUT = 2000;
const int MIN_MOTOR_DIR_OUTPUT = 1000;
const int MAX_DIR_INPUT        = 1592;
const int MIN_DIR_INPUT        = 772;


// 25 milliseconds
const int PULSEIN_TIMEOUT = 25000;

/**
 * Channel 3 = Lift
 * Channel 2 = Thrust forward
 * Channel 1 = Direction
 */


// Channel 0: 772 - 1592 (dir)
// Channel 1: 760 - 1584 (thrust) (mid = 1180)
// Channel 2: 572 - 1400 (lift)
// Channel 3: 764 - 1584 (not used)
// Channel 4: 580, 1380
volatile uint16_t curPulse = 0;
volatile uint32_t PULSESTART = 0;
volatile uint32_t PULSES[5] = {0,0,0,0,0};

const int FRAME_SIZE = 5;
int CHANNEL_FILTER[FRAME_SIZE] = {1500};

Servo thrust_servo;

void sort(int* data, const int len) {
  for (int i = 1; i < len; ++i) {
    int j = data[i];
    int k;
    for (k = i - 1; (k >= 0) && (j > data[k]); k--) {
      data[k + 1] = data[k];
    }
    data[k + 1] = j;
  }
}

int median_filter(int data_point, int* data) {
  static int sorted_data[FRAME_SIZE];
  for (int i = FRAME_SIZE - 1; i > 0; i--) {
    data[i] = data[i - 1];
  }
  //add in new value
  data[0] = data_point;

  // Create a sorted array
  memcpy(sorted_data, data, FRAME_SIZE * sizeof(int));
  sort(sorted_data, FRAME_SIZE);

  // Get median
  return sorted_data[FRAME_SIZE / 2];
}

// Returns the pulse length (in microseconds) on pin "pin"
// Returns 0 if timeout
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  uint8_t stateMask = (state ? bit : 0);
  unsigned long width = 0;
  
  unsigned long numloops = 0;
  unsigned long maxloops = microsecondsToClockCycles(timeout);
  
  while ((*portInputRegister(port) & bit) == stateMask) {
    if (numloops++ == maxloops) {
      return 0;
    }
  }
  
  while ((*portInputRegister(port) & bit) != stateMask) {
    if (numloops++ == maxloops) {
      return 0;
    }
  }
  
  unsigned long start = micros();
  while ((*portInputRegister(port) & bit) == stateMask) {
    if (numloops++ == maxloops) {
      return 0;
    }
  }
  return micros() - start;
}

void ISRPIN2(){
  // Handle PPM pin changes
   if(digitalRead(PPM_PIN) == HIGH){
     //rising edge
     PULSESTART = micros();
   }else{
     //falling edge
     uint16_t time = micros() - PULSESTART;
     if(time > 5000){
       // Greater than 5ms, therefore it's the gap.
       curPulse = 0;
       return;
     }
     if(curPulse > 4){
       // we have a problem, controller disconnected?
       // continue current state
       return; 
     }
     // Assign the pulse and increment the count.
     PULSES[curPulse++] = time;
   }
}

void setup(){
  
  //start serial connection
  Serial.begin(9600);

  // @TODO: Needs to pulse at 1Hz  
  pinMode(LED_PIN, OUTPUT);  
  pinMode(THRUST_DIR_PIN, OUTPUT);
  digitalWrite(THRUST_DIR_PIN, LOW);
  
  // Setup lift and thrust output pins
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  pinMode(THRUST_MOTOR_PIN, OUTPUT);
  pinMode(PPM_PIN, INPUT);
  
  thrust_servo.attach(MOTOR_SERVO_PIN);
  thrust_servo.writeMicroseconds(1500);
  
  attachInterrupt(0,ISRPIN2,CHANGE);
}

void lift() {
  //int lift = pulseInLong(CHANNEL_PINS[LIFT_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  int lift = PULSES[2];
  if (lift != 0) {
    lift = constrain(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT);
    lift = map(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT, MIN_LIFT_OUTPUT, MAX_LIFT_OUTPUT);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
}

void thrust() {
   // Read thrust from receiver and output to motor  
  //int thrust = pulseInLong(CHANNEL_PINS[THRUST_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  int thrust = PULSES[1];
  if (thrust != 0) {
    if (thrust < MID_THRUST_INPUT - 40) {
      thrust = map(thrust, MID_THRUST_INPUT, MIN_THRUST_INPUT, MID_THRUST_INPUT, MAX_THRUST_INPUT);
      digitalWrite(THRUST_DIR_PIN, HIGH);
    } else {
      digitalWrite(THRUST_DIR_PIN, LOW);
    }
    thrust = constrain(thrust, MID_THRUST_INPUT, MAX_THRUST_INPUT);
    thrust = map(thrust, MID_THRUST_INPUT, MAX_THRUST_INPUT, MIN_THRUST_OUTPUT, MAX_THRUST_OUTPUT);
  }
  analogWrite(THRUST_MOTOR_PIN, thrust);
}

void boost() {
  //int boost = pulseInLong(CHANNEL_PINS[BOOST_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  int boost = PULSES[4];
  
  if (boost < 1500) {
    MAX_LIFT_OUTPUT = 255;
    digitalWrite(LED_PIN, HIGH);
  } else {
    MAX_LIFT_OUTPUT = 200;
    digitalWrite(LED_PIN, LOW);
  }
}

void dir() {
  // Read in direction from receiver and output to both servos
  //int dir = pulseInLong(CHANNEL_PINS[DIR_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  int dir = PULSES[0];
  int thrust_dir;
  if (dir != 0) {
    dir = constrain(dir, MIN_DIR_INPUT, MAX_DIR_INPUT);
    thrust_dir  = map(dir, MIN_DIR_INPUT, MAX_DIR_INPUT, MAX_MOTOR_DIR_OUTPUT, MIN_MOTOR_DIR_OUTPUT);
  } else {
    thrust_dir = 1500;
  }
  thrust_servo.writeMicroseconds(thrust_dir);
  
}

void loop(){
  // Read lift from the reciever and output to motor
  //boost();
  lift();
  thrust();
  dir();
  if (micros() - PULSESTART > 30000) {
    for (int i = 0; i < 5; i++) {
      PULSES[i] = 0;
    }
  }
  
  delay(25);
  
}
