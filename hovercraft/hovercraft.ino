#include <Servo.h>

const int LIFT_MOTOR_PIN = A0;
const int PULL_MOTOR_PIN = A1;
const int MOTOR_SERVO_PIN = 6;
const int RUDDER_SERVO_PIN = 5;
const int POT_PIN = A1;
const int LED_PIN = 13;
const int TIMER_INTERRUPT = 0;
const byte MOTOR_ON  = 255;
const byte MOTOR_OFF = 0;
const int ANALOG_READ_MAX = 1023;
const int ANALOG_READ_MIN = 0;

// 25 milliseconds
const int PULSEIN_TIMEOUT = 25000;

/**
 * Channel 3 = Lift
 * Channel 2 = Pull (Thrust forward)
 * Channel 1 = Direction
 */
const int CHANNEL_PINS [4] = {
  -1, /* Channel 0 does not exist */
  7,
  8,
  9,
};

Servo pull;
unsigned last_pull_us = 1500;
Servo rudder;
unsigned last_rudder_us = 1500;

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

void setup(){
  
  //start serial connection
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  pinMode(PULL_MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  for (int i = 1; i < (sizeof(CHANNEL_PINS) / sizeof(CHANNEL_PINS[0]));  i++) {
    pinMode(CHANNEL_PINS[i], INPUT);
  }
  
  pull.attach(MOTOR_SERVO_PIN);
  pull.writeMicroseconds(1500);
  rudder.attach(RUDDER_SERVO_PIN);
  rudder.writeMicroseconds(1500);
}

void loop(){
  // Read from the reciever
  int lift = pulseInLong(CHANNEL_PINS[3], HIGH, PULSEIN_TIMEOUT);
  if (lift != 0) {
    lift = map(constrain(lift, 1000, 1850), 1000, 1850, 0, 255);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
    
  int thrust = pulseInLong(CHANNEL_PINS[2], HIGH, PULSEIN_TIMEOUT);
  if (thrust != 0) {
    thrust = map(constrain(thrust, 1500, 1950), 1500, 1950, 0, 255);
  }
  analogWrite(PULL_MOTOR_PIN, thrust);
  
  int dir = pulseInLong(CHANNEL_PINS[1], HIGH, PULSEIN_TIMEOUT);
  if (dir != 0) {
    dir = map(constrain(dir, 1060, 1860), 1060, 1860, 900, 2100);
  } else {
    dir = 1500;
  }
  
  pull.writeMicroseconds(dir);
  rudder.writeMicroseconds(dir);
  
  Serial.print("Millis: ");
  Serial.print(millis());
  Serial.print(" Lift: ");
  Serial.print(lift);
  Serial.print(" Pull: ");
  Serial.print(thrust);
  Serial.print(" Dir: ");
  Serial.print(dir);
  Serial.print("\n");
}

void turn_off_motor() {
  analogWrite(LIFT_MOTOR_PIN, MOTOR_OFF);
}
