#include <Servo.h>

const int LIFT_MOTOR_PIN = A0;
const int PULL_MOTOR_PIN = A2;
const int MOTOR_SERVO_PIN = 9;
const int RUDDER_SERVO_PIN = 10;
const int POT_PIN = A1;
const int LED_PIN = 13;
const int TIMER_INTERRUPT = 0;
const byte MOTOR_ON  = 255;
const byte MOTOR_OFF = 0;
const int ANALOG_READ_MAX = 1023;
const int ANALOG_READ_MIN = 0;
const int CHANNEL_PINS [5] = {
  -1, /* Channel 0 does not exist */
  4,
  5,
  6,
  7
};

Servo pull;
Servo rudder;

void setup(){
  
  //start serial connection
#ifdef DEBUG
  Serial.begin(9600);
#endif
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  pinMode(PULL_MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  for (int i = 0; i < (sizeof(CHANNEL_PINS) / sizeof(CHANNEL_PINS[0]));  i++) {
    pinMode(CHANNEL_PINS[i], INPUT);
  }
  
  pull.attach(MOTOR_SERVO_PIN);
  pull.write(90);
  rudder.attach(RUDDER_SERVO_PIN);
  rudder.write(90);
}

void loop(){
  analogWrite(LIFT_MOTOR_PIN, MOTOR_ON);
}

void turn_off_motor() {
  analogWrite(LIFT_MOTOR_PIN, MOTOR_OFF);
}
