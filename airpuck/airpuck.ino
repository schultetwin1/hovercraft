const int LIFT_MOTOR_PIN = A0;
const int PULL_MOTOR_PIN = A2;
const int POT_PIN = A1;
const int BUTTON_PIN = 2;
const int LED_PIN = 13;
const int TIMER_INTERRUPT = 0;
const byte MOTOR_ON  = 255;
const byte MOTOR_OFF = 0;
const int ANALOG_READ_MAX = 1023;
const int ANALOG_READ_MIN = 0;

void setup(){
  //start serial connection
#ifdef DEBUG
  Serial.begin(9600);
#endif
  
  // configure pin2 as an input and enable the internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LIFT_MOTOR_PIN, OUTPUT);
  pinMode(PULL_MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);  
}

void loop(){
  //read the pushbutton value into a variable
  int buttonVal = digitalRead(BUTTON_PIN);
  
  if (buttonVal == HIGH) {
    // Button not pressed
    digitalWrite(LED_PIN, LOW);
  } else {
    // Button pressed
    analogWrite(LIFT_MOTOR_PIN, MOTOR_ON);
    digitalWrite(LED_PIN, HIGH);
  }
}

void turn_off_motor() {
  analogWrite(LIFT_MOTOR_PIN, MOTOR_OFF);
}
