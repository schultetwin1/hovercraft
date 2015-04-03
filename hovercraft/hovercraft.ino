#include <Servo.h>
#include "PPM.h"

// These pin numbers have been choose carefully
// Please ask Matt before changing
// DO NOT USE analogWrite on pins 9 or 10, it will not work
// See (http://arduino.cc/en/reference/servo) for explination

// Pin 3 and Pin 11 use Timer 2
// Pin 2 used by PPM library
const int LIFT_MOTOR_PIN   =  3;
const int MOTOR_SERVO_PIN  =  6;
const int THRUST_DIR_PIN   = 12;
const int THRUST_MOTOR_PIN = 11;
const int LED_PIN          = 13;

/**
 * The following min and max inputs come from the oscope.
 * If you play with the trim, you mess up these numbers
 * DO NOT PLAY WITH THE TRIM
 */
// Output between 0 - 200 (not 255 to save the motor)
int       MAX_LIFT_OUTPUT =  200;
const int MIN_LIFT_OUTPUT =    0;
const int MAX_LIFT_INPUT  = 1160;
const int MIN_LIFT_INPUT  =  572;

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

// Channel 0: 772 - 1592 (dir)
// Channel 1: 760 - 1584 (thrust) (mid = 1180)
// Channel 2: 572 - 1400 (lift)
// Channel 3: 764 - 1584 (not used)
// Channel 4: 580, 1380
const int DIR_CHANNEL = 0;
const int THRUST_CHANNEL = 1;
const int LIFT_CHANNEL = 2;
const int BOOST_CHANNEL = 4;

Servo thrust_servo;

void setup(){

  // @TODO: Needs to pulse at 1Hz  
  pinMode(LED_PIN, OUTPUT);
  
  // Set thrust direction output
  // Initialize to forward
  pinMode(THRUST_DIR_PIN, OUTPUT);
  digitalWrite(THRUST_DIR_PIN, LOW);
  
  // Setup lift and thrust output pins
  // Also 0 outputs
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  analogWrite(LIFT_MOTOR_PIN, 0);
  pinMode(THRUST_MOTOR_PIN, OUTPUT);
  analogWrite(THRUST_MOTOR_PIN, 0);
  
  // Setup servo
  // Move servo to center
  thrust_servo.attach(MOTOR_SERVO_PIN);
  thrust_servo.writeMicroseconds(1500);
  
  // Setup PPM receiver
  PPM::begin();
}

void lift() {
  int lift = PPM::channelPulse(LIFT_CHANNEL);
  if (lift != 0) {
    lift = constrain(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT);
    lift = map(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT, MIN_LIFT_OUTPUT, MAX_LIFT_OUTPUT);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
}

void thrust() {
   // Read thrust from receiver and output to motor  
  int thrust = PPM::channelPulse(THRUST_CHANNEL);
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
  int boost = PPM::channelPulse(BOOST_CHANNEL);
  
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
  int dir = PPM::channelPulse(DIR_CHANNEL);
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
  boost();
  lift();
  thrust();
  dir();
  if (!PPM::controllerConnected()) {
    PPM::zeroPulses();
  }
  delay(25);
  
}
