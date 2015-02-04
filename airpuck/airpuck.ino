#include <Event.h>
#include <Timer.h>
 

const int MOTOR_PIN = A0;
const int POT_PIN = A1;
const int BUTTON_PIN = 2;
const int LED_PIN = 13;
const int TIMER_INTERRUPT = 0;
const byte MOTOR_ON  = 255;
const byte MOTOR_OFF = 0;
const int ANALOG_READ_MAX = 1023;
const int ANALOG_READ_MIN = 0;

Timer t;
static int lastest_timer_event = -1;

void start_timer() {
  int val = analogRead(POT_PIN);
  // If POT is al the way, run forever
  if (val < 1020) {
    latest_timer_event = t.after(map(val, ANALOG_READ_MIN, ANALOG_READ_MAX, 4000,  20000), turn_off_motor);
  }
}

void setup(){
  //start serial connection
#ifdef DEBUG
  Serial.begin(9600);
#endif
  
  // configure pin2 as an input and enable the internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);  
  attachInterrupt(TIMER_INTERRUPT, start_timer, RISING);
}

void loop(){
  t.update();
  //read the pushbutton value into a variable
  int sensorVal = digitalRead(BUTTON_PIN);
  //print out the value of the pushbutton
#ifdef DEBUG
  Serial.println(sensorVal);
#endif
  
  // Keep in mind the pullup means the pushbutton's
  // logic is inverted. It goes HIGH when it's open,
  // and LOW when it's pressed. Turn on pin 13 when the 
  // button's pressed, and off when it's not:
  if (sensorVal == HIGH) {
    digitalWrite(LED_PIN, LOW);
  } else {
    analogWrite(MOTOR_PIN, MOTOR_ON);
    digitalWrite(LED_PIN, HIGH);
    if (latest_timer_event != -1) {
      t.stop(latest_timer_event);
      latest_timer_event = -1;
    }
  }
}

void turn_off_motor() {
  analogWrite(MOTOR_PIN, MOTOR_OFF);
}


