#include <Servo.h>

// These pin numbers have been choose carefully
// Please ask Matt before changing
// DO NOT USE analogWrite on pins 9 or 10, it will not work
// See (http://arduino.cc/en/reference/servo) for explination

// Pin 3 and Pin 11 use Timer 2
const int LIFT_MOTOR_PIN = 3;
const int PULL_MOTOR_PIN = 11;

const int MOTOR_SERVO_PIN = 6;
const int RUDDER_SERVO_PIN = 5;

const int LED_PIN = 13;

const int MAX_LIFT_OUTPUT = 200;
const int MIN_LIFT_OUTPUT = 0;
const int MAX_LIFT_INPUT  = 1850;
const int MIN_LIFT_INPUT  = 1000;

const int MAX_THRUST_OUTPUT = 255;
const int MIN_THRUST_OUTPUT = 0;
const int MAX_THRUST_INPUT  = 1900;
const int MIN_THRUST_INPUT  = 1550;

const int MAX_RUDDER_OUTPUT    = 2100;
const int MIN_RUDDER_OUTPUT    = 900;
const int MAX_MOTOR_DIR_OUTPUT = 2100;
const int MIN_MOTOR_DIR_OUTPUT = 900;
const int MAX_DIR_INPUT        = 1860;
const int MIN_DIR_INPUT        = 1060;


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

const int FRAME_SIZE = 5;
int CHANNEL_FILTER[FRAME_SIZE] = {1500};

Servo pull;
Servo rudder;

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
    lift = constrain(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT);
    lift = map(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT, MIN_LIFT_OUTPUT, MAX_LIFT_OUTPUT);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
    
  int thrust = pulseInLong(CHANNEL_PINS[2], HIGH, PULSEIN_TIMEOUT);
  if (thrust != 0) {
    thrust = constrain(thrust, MIN_THRUST_INPUT, MAX_THRUST_INPUT);
    thrust = map(thrust, MIN_THRUST_INPUT, MAX_THRUST_INPUT, MIN_THRUST_OUTPUT, MAX_THRUST_OUTPUT);
  }
  analogWrite(PULL_MOTOR_PIN, thrust);
  
  int dir = pulseInLong(CHANNEL_PINS[1], HIGH, PULSEIN_TIMEOUT);
  int rudder_dir;
  int motor_dir;
  if (dir != 0) {
    dir = constrain(dir, MIN_DIR_INPUT, MAX_DIR_INPUT);
    rudder_dir = map(dir, MIN_DIR_INPUT, MAX_DIR_INPUT, MIN_RUDDER_OUTPUT, MAX_RUDDER_OUTPUT);
    motor_dir  = map(dir, MIN_DIR_INPUT, MAX_DIR_INPUT, MIN_MOTOR_DIR_OUTPUT, MAX_MOTOR_DIR_OUTPUT);
  } else {
    dir = 1500;
  }
  
  pull.writeMicroseconds(motor_dir);
  rudder.writeMicroseconds(rudder_dir);
  
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
