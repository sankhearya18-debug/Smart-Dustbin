#include <Servo.h>

#define TRIG_PIN   D6
#define ECHO_PIN   D7

#define SERVO_PIN  D4

#define RED_LED    D2
#define GREEN_LED  D1

Servo dustbinServo;

// SG90 positions
const int CLOSED_ANGLE = 10;
const int OPEN_ANGLE   = 130;

// Ultrasonic detection
const int DETECT_DISTANCE = 4;
const int RESET_DISTANCE  = 30;

// Open for 5 seconds
const unsigned long OPEN_TIME = 5000;

// Red blink speed
const unsigned long BLINK_TIME = 100;

bool binOpen = false;
bool readyForNewHand = true;

unsigned long openStartTime = 0;
unsigned long lastBlinkTime = 0;

bool redState = false;


// --------------------------------------------------
// GET DISTANCE
// --------------------------------------------------

long getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  long distance = duration * 0.0343 / 2;

  return distance;
}


// --------------------------------------------------
// OPEN BIN
// --------------------------------------------------

void openBin() {

  Serial.println("Opening bin...");

  dustbinServo.write(OPEN_ANGLE);

  binOpen = true;
  readyForNewHand = false;

  openStartTime = millis();

  digitalWrite(GREEN_LED, LOW);

  redState = true;
  digitalWrite(RED_LED, HIGH);

  lastBlinkTime = millis();
}


// --------------------------------------------------
// CLOSE BIN
// --------------------------------------------------

void closeBin() {

  Serial.println("Closing bin...");

  dustbinServo.write(CLOSED_ANGLE);

  binOpen = false;

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
}


// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);

  // SG90 attach with safe pulse range
  dustbinServo.attach(SERVO_PIN, 500, 2400);

  // Start position
  dustbinServo.write(CLOSED_ANGLE);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  delay(500);

  Serial.println("================================");
  Serial.println(" SMART DUSTBIN READY");
  Serial.println("================================");
}


// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop() {

  long distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");


  // ==================================================
  // BIN CLOSED
  // ==================================================

  if (!binOpen) {

    digitalWrite(GREEN_LED, HIGH);

    // Hand detected
    if (readyForNewHand && distance <= DETECT_DISTANCE) {

      Serial.println("Hand detected!");

      openBin();
    }

    // Hand moved away
    if (!readyForNewHand && distance >= RESET_DISTANCE) {

      readyForNewHand = true;

      Serial.println("Ready for next hand.");
    }
  }


  // ==================================================
  // BIN OPEN
  // ==================================================

  if (binOpen) {

    // Fast red blink
    if (millis() - lastBlinkTime >= BLINK_TIME) {

      lastBlinkTime = millis();

      redState = !redState;

      digitalWrite(RED_LED, redState);
    }


    // 5 seconds complete
    if (millis() - openStartTime >= OPEN_TIME) {

      closeBin();
    }
  }

  delay(50);
}