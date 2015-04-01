#include <Servo.h>

// These pin numbers have been choose carefully
// Please ask Matt before changing
// DO NOT USE analogWrite on pins 9 or 10, it will not work
// See (http://arduino.cc/en/reference/servo) for explination

// Pin 3 and Pin 11 use Timer 2
const int LIFT_MOTOR_PIN = 3;
const int THRUST_MOTOR_PIN = 11;
const int BOOST_PIN = 12;

const int MOTOR_SERVO_PIN = 6;

const int LED_PIN = 13;

// Lift Input will be between 1000us and 1850us
// Output between 0 - 200 (not 255 to save the motor)
int MAX_LIFT_OUTPUT = 200;
const int MIN_LIFT_OUTPUT = 0;
const int MAX_LIFT_INPUT  = 1850;
const int MIN_LIFT_INPUT  = 1050;

// Thrust Input will be between 1550us (halfway) and 1900us
// Output between 0 - 255
const int MAX_THRUST_OUTPUT = 255;
const int MIN_THRUST_OUTPUT = 0;
const int MAX_THRUST_INPUT  = 1900;
const int MIN_THRUST_INPUT  = 1600;

// Lift Input will be between 1060 and 1860us
// Output between 900 - 2100 us
const int MAX_MOTOR_DIR_OUTPUT = 2000;
const int MIN_MOTOR_DIR_OUTPUT = 1000;
const int MAX_DIR_INPUT        = 1860;
const int MIN_DIR_INPUT        = 1060;


// 25 milliseconds
const int PULSEIN_TIMEOUT = 25000;

/**
 * Channel 3 = Lift
 * Channel 2 = Thrust forward
 * Channel 1 = Direction
 */

enum Channels {
  LIFT_CHANNEL = 3,
  THRUST_CHANNEL = 2,
  DIR_CHANNEL = 1,
  BOOST_CHANNEL = 5
};  
  
const int CHANNEL_PINS [6] = {
  -1, /* Channel 0 does not exist */
  7,
  8,
  9,
  -1, /* Not using channel 4 */
  12
};

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

void setup(){
  
  //start serial connection
  Serial.begin(9600);

  // @TODO: Needs to pulse at 1Hz  
  pinMode(LED_PIN, OUTPUT);
  
  // Setup lift and thrust output pins
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  pinMode(THRUST_MOTOR_PIN, OUTPUT);
  for (int i = 1; i < (sizeof(CHANNEL_PINS) / sizeof(CHANNEL_PINS[0]));  i++) {
    pinMode(CHANNEL_PINS[i], INPUT);
  }
  
  thrust_servo.attach(MOTOR_SERVO_PIN);
  thrust_servo.writeMicroseconds(1500);
}

void loop(){
  // Read lift from the reciever and output to motor
  int all_read = 1;
  int lift = pulseInLong(CHANNEL_PINS[LIFT_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  Serial.print(" Lift: ");
  Serial.print(lift);
  if (lift != 0) {
    lift = constrain(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT);
    lift = map(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT, MIN_LIFT_OUTPUT, MAX_LIFT_OUTPUT);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
  
  
  // Read thrust from receiver and output to motor  
  int thrust = pulseInLong(CHANNEL_PINS[THRUST_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  Serial.print(" Thrust: ");
  Serial.print(thrust);
  if (thrust != 0) {
    thrust = constrain(thrust, MIN_THRUST_INPUT, MAX_THRUST_INPUT);
    thrust = map(thrust, MIN_THRUST_INPUT, MAX_THRUST_INPUT, MIN_THRUST_OUTPUT, MAX_THRUST_OUTPUT);
  }
  analogWrite(THRUST_MOTOR_PIN, thrust);
  
  // Read in direction from receiver and output to both servos
  int dir = pulseInLong(CHANNEL_PINS[DIR_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  int thrust_dir;
  if (dir != 0) {
    dir = constrain(dir, MIN_DIR_INPUT, MAX_DIR_INPUT);
    thrust_dir  = map(dir, MIN_DIR_INPUT, MAX_DIR_INPUT, MAX_MOTOR_DIR_OUTPUT, MIN_MOTOR_DIR_OUTPUT);
  } else {
    thrust_dir = 1500;
  }
  
  thrust_servo.writeMicroseconds(thrust_dir);
  
  int boost = pulseInLong(CHANNEL_PINS[BOOST_CHANNEL], HIGH, PULSEIN_TIMEOUT);
  
  if (boost < 1500) {
    MAX_LIFT_OUTPUT = 255;
    digitalWrite(LED_PIN, HIGH);
  } else {
    MAX_LIFT_OUTPUT = 200;
    digitalWrite(LED_PIN, LOW);
  }
  
  Serial.print(" Boost: ");
  Serial.print(boost);
  Serial.print("\n");
}
