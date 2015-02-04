#include <Event.h>
#include <Timer.h>

/*
 Input Pullup Serial
 
 This example demonstrates the use of pinMode(INPUT_PULLUP). It reads a 
 digital input on pin 2 and prints the results to the serial monitor.
 
 The circuit: 
 * Momentary switch attached from pin 2 to ground 
 * Built-in LED on pin 13
 
 Unlike pinMode(INPUT), there is no pull-down resistor necessary. An internal 
 20K-ohm resistor is pulled to 5V. This configuration causes the input to 
 read HIGH when the switch is open, and LOW when it is closed. 
 
 created 14 March 2012
 by Scott Fitzgerald
 
 http://www.arduino.cc/en/Tutorial/InputPullupSerial
 
 This example code is in the public domain
 
 */
 

const int MOTOR_PIN = A0;
const int POT_PIN = A1;
const int BUTTON_PIN = 2;
const int LED_PIN = 13;
const int TIMER_INTERRUPT = 0;
const byte MOTOR_ON  = 255;
const byte MOTOR_OFF = 0;

Timer t;

void start_timer() {
  int val = analogRead(POT_PIN);
  if (val < 1020) {
    t.after(map(val, 0, 1023, 4000,  20000), turn_off_motor);
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
  }
}

void turn_off_motor() {
  analogWrite(MOTOR_PIN, MOTOR_OFF);
}


