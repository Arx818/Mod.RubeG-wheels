#include <NewPing.h>
#include <ServoTimer2.h>
#include <NewTone.h>

//define board pins, servo positions, hc-sr04 maximum distance, number of leds pins and buzzer frequencies.
#define BUTTON_PIN 2
#define SWITCH_PIN 3
#define TRIG_PIN 4
#define ECHO_PIN 5
#define BUZZER_PIN 6
#define WHITE_LED_PIN 10
#define RED_LED_PIN 11
#define GREEN_LED_PIN 12
#define LED_TOT_PINS 3
#define SERVO_PIN 9
#define SERVO_DOWN 750
#define SERVO_UP 1500
#define MAX_DISTANCE 10
#define COUNTDOWN_FREQUENCY 1000
#define ARRIVED_FREQUENCY 1200
#define ERROR_FREQUENCY 2000

//states of the finite state machine
enum State {
  IDLE_STATE,
  START_STATE,
  WAIT_FOR_CAR_STATE,
  CAR_ARRIVED_STATE,
  ERROR_STATE
};

//variables declaration
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
State currentState = IDLE_STATE;
bool servoPositioned = true;  //true if servo is in up position
ServoTimer2 myservo;
uint8_t ledPins[LED_TOT_PINS] = { 11, 10, 12 };
unsigned long startTime;

//functions prototypes
void start();
void customStateBehavior(uint8_t ledPin, unsigned long frequency, unsigned long stateTime);

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  myservo.attach(9);
  myservo.write(SERVO_UP);            //set initial position of servo
  digitalWrite(WHITE_LED_PIN, HIGH);  //turn on white led
  //Serial.begin(115200);             //uncomment for serial communication
}

void start() {
  for (uint8_t i = 0; i < LED_TOT_PINS - 1; i++) {  //turn on each led, skipping the green one that is already on
    digitalWrite(ledPins[i], HIGH);
  }
  for (uint8_t i = 0; i < LED_TOT_PINS; i++) {  //turn off each led with some delay between each one, with a sound effect
    delay(500);
    NewTone(BUZZER_PIN, COUNTDOWN_FREQUENCY);
    delay(500);
    noNewTone(BUZZER_PIN);
    digitalWrite(ledPins[i], LOW);
  }
  myservo.write(SERVO_DOWN);  //servo lets the car go
  servoPositioned = false;
}

void customStateBehavior(uint8_t ledPin, unsigned long frequency, unsigned long stateTime) {
  while (millis() - stateTime < 4000) {  //stay in the state (and in function) for 4 seconds
    digitalWrite(ledPin, HIGH);          //led blinks (red led -> error state; green led -> car arrived state)
    NewTone(BUZZER_PIN, frequency);      //sound effect (2000 -> error state; 1200 -> car arrived state)
    delay(250);
    noNewTone(BUZZER_PIN);
    digitalWrite(ledPin, LOW);
    delay(250);
  }
}

void loop() {
  switch (currentState) {
    case IDLE_STATE:
      if (!servoPositioned) {               //if i come from an another state
        myservo.write(SERVO_UP);            //put servo in up position
        digitalWrite(WHITE_LED_PIN, HIGH);  //turn on white led
        servoPositioned = true;
      }
      if (digitalRead(BUTTON_PIN) == LOW && servoPositioned) {  //button pressed to confirm the presence of the car
        currentState = START_STATE;
      }
      break;
    case START_STATE:
      digitalWrite(WHITE_LED_PIN, LOW);      //turn off white led
      digitalWrite(GREEN_LED_PIN, HIGH);     //turn on green led
      if (digitalRead(SWITCH_PIN) == LOW) {  //input from switch
        start();                             //activate start sequence
        startTime = millis();                //set start event time
        currentState = WAIT_FOR_CAR_STATE;
      }
      break;
    case WAIT_FOR_CAR_STATE:
      if (millis() - startTime > 4000)  //If the car is not detected within 4 seconds
        currentState = ERROR_STATE;
      //Serial.println(sonar.ping_cm()); //uncomment for serial communication
      if (sonar.ping_cm() >= 1 && sonar.ping_cm() <= 8)  //if the car is detected
        currentState = CAR_ARRIVED_STATE;
      break;
    case CAR_ARRIVED_STATE:
      customStateBehavior(GREEN_LED_PIN, ARRIVED_FREQUENCY, millis());
      currentState = IDLE_STATE;
      break;
    case ERROR_STATE:
      customStateBehavior(RED_LED_PIN, ERROR_FREQUENCY, millis());
      currentState = IDLE_STATE;
      break;
  }
}