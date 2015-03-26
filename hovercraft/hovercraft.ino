#include <Servo.h>

const int LIFT_MOTOR_PIN = 10;
const int PULL_MOTOR_PIN = 11;
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
  int lift = pulseIn(CHANNEL_PINS[3], HIGH, PULSEIN_TIMEOUT);
  int thrust = pulseIn(CHANNEL_PINS[2], HIGH, PULSEIN_TIMEOUT);
  int dir = pulseIn(CHANNEL_PINS[1], HIGH, PULSEIN_TIMEOUT);
  dir = map(constrain(dir, 1060, 1860), 1060, 1860, 900, 2100);
  lift = map(constrain(lift, 1000, 1850), 1000, 1850, 0, 255);
  thrust = map(constrain(thrust, 1500, 1950), 1500, 1950, 0, 255);
  
  Serial.print("Millis: ");
  Serial.print(millis());
  Serial.print(" Lift: ");
  Serial.print(lift);
  Serial.print(" Pull: ");
  Serial.print(thrust);
  Serial.print(" Dir: ");
  Serial.print(dir);
  Serial.print("\n");
  
  
  analogWrite(LIFT_MOTOR_PIN, lift);
  analogWrite(PULL_MOTOR_PIN, thrust);
  pull.writeMicroseconds(dir);
  rudder.writeMicroseconds(dir);

}

void turn_off_motor() {
  analogWrite(LIFT_MOTOR_PIN, MOTOR_OFF);
}
