#include <Servo.h>
#include "PPM.h"

/**
 * NEED TO TRY THIS:
 * http://letsmakerobots.com/content/wireless-upload-link-arduino-using-xbee-modules
 */

// These pin numbers have been choose carefully
// Please ask Matt before changing
// DO NOT USE analogWrite on pins 9 or 10, it will not work
// See (http://arduino.cc/en/reference/servo) for explination

// Timer2 used by analogWrite to pin 3 and pin 11
// Timer1 used by Servo library
// Timer0 used by millis() and micros()

// @TODO: Switch pwm from analogWrite to pins 5 & 6
// @TODO: Rewrite micros() using timer2 to get better accuracy (~0.5us)
// Pin 3 and Pin 11 use Timer 2
// Pin 2 used by PPM library
const int LIFT_MOTOR_PIN   =  3;
const int MOTOR_SERVO_PIN  =  6;
const int THRUST_DIR_PIN   = 12;
const int THRUST_MOTOR_PIN = 11;
const int BOOST_LED_PIN    = 13;
const int STATUS_LED_PIN   = 9;

/**
 * The following min and max inputs come from observations on the oscope.
 * If you play with the trim, you mess up these numbers
 * DO NOT PLAY WITH THE TRIM
 */
// Output between 0 - 255
const int MAX_LIFT_OUTPUT = 255;
const int MIN_LIFT_OUTPUT = 0;
const int MAX_LIFT_INPUT  = 1392;
const int MIN_LIFT_INPUT  = 436;

// Output between 0 - 255
// Use mid_input to reverse motor
const int MAX_THRUST_OUTPUT = 255;
const int MIN_THRUST_OUTPUT = 0;
const int MAX_THRUST_INPUT  = 1424;
const int MIN_THRUST_INPUT  = 600;
const int MID_THRUST_INPUT  = 1016;

// Lift Input will be between 1060 and 1860us
// Output between 1000 - 2000 us
const int MAX_MOTOR_DIR_OUTPUT = 2200;
const int MIN_MOTOR_DIR_OUTPUT = 1200;
const int MID_MOTOR_DIR_OUTPUT = (MIN_MOTOR_DIR_OUTPUT + MAX_MOTOR_DIR_OUTPUT) / 2;
const int MAX_DIR_INPUT        = 1564;
const int MIN_DIR_INPUT        = 728;

// Channel 0: 772 - 1592 (dir)
// Channel 1: 760 - 1584 (thrust) (mid = 1180)
// Channel 2: 572 - 1400 (lift)
// Channel 3: 764 - 1584 (not used)
// Channel 4: 580, 1380
const int DIR_CHANNEL = 0;
const int THRUST_CHANNEL = 1;
const int LIFT_CHANNEL = 2;
const int BOOST_CHANNEL = 4;

static bool is_boosted = false;
Servo thrust_servo;

void setup(){

  //Serial.begin(9600);
  pinMode(BOOST_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
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
  thrust_servo.writeMicroseconds(MID_MOTOR_DIR_OUTPUT);
  
  // Setup PPM receiver
  PPM::begin();
}

// Reads in lifts pulse and outputs the correct lift to the motor
void lift() {
  int lift = PPM::channelPulse(LIFT_CHANNEL);

  if (lift != 0) {
    lift = constrain(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT);
    lift = map(lift, MIN_LIFT_INPUT, MAX_LIFT_INPUT, MIN_LIFT_OUTPUT, MAX_LIFT_OUTPUT);
  }
  if (!is_boosted) {
    lift = constrain(lift, MIN_LIFT_OUTPUT, 200);
  }
  analogWrite(LIFT_MOTOR_PIN, lift);
}

// Reads in thrust pulse and outputs the correct lift to the motor
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

// Reads in channel 5's boost and decides if boost should be enabled
void boost() {
  int boost = PPM::channelPulse(BOOST_CHANNEL);

  if (boost < 1200) {
    is_boosted = true;
    digitalWrite(BOOST_LED_PIN, HIGH);
  } else {
    is_boosted = false;
    digitalWrite(BOOST_LED_PIN, LOW);
  }
}

// Reads in thrust pulse and outputs the correct direction to the servo
void dir() {
  // Read in direction from receiver and output to both servos
  int dir = PPM::channelPulse(DIR_CHANNEL);

  int thrust_dir;
  if (dir != 0) {
    dir = constrain(dir, MIN_DIR_INPUT, MAX_DIR_INPUT);
    thrust_dir  = map(dir, MIN_DIR_INPUT, MAX_DIR_INPUT, MAX_MOTOR_DIR_OUTPUT, MIN_MOTOR_DIR_OUTPUT);
  } else {
    thrust_dir = MID_MOTOR_DIR_OUTPUT;
  }
  thrust_servo.writeMicroseconds(thrust_dir);
  
}

void loop(){
  if (!PPM::controllerConnected()) {
    PPM::zeroPulses();
  }
  // Read lift from the reciever and output to motor
  boost();
  lift();
  thrust();
  dir();
  
  
  if (millis() % 1000 < 500) {
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }
  delay(20);
  
}
