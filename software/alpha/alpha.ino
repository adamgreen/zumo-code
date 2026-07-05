// 2026 - Adam Green (https://github.com/adamgreen)
// Mini-Sumo Firmware for the Pololu Zumo 2040 Robot
// Version: Alpha
//
// Notes:
// ------
// This is an Arduino port of the SumoBot-5.1-Basic-Competition-Program
// BASIC Stamp 2 code from Parallax's SumoBot documentation.
//
// I plan to use this as one of the firmware versions that I test my
// Mini-Sumo software creations against in additions to the ones that
// Pololu includes with their Zumo Arduino Library.
#include <Zumo2040.h>

// Motor Left/Right Forward/Stop/Reverse Slow/Fast Values.
const int16_t MOTOR_FWD_FAST = 400;
const int16_t MOTOR_FWD_SLOW = 150;
const int16_t MOTOR_STOP = 0;
const int16_t MOTOR_REV_SLOW = -150;
const int16_t MOTOR_REV_FAST = -400;

// Ordinals of left and rightmost sensors in g_lineSensors.rawSensorValues[].
const size_t LEFT_SENSOR = 0;
const size_t RIGHT_SENSOR = LINE_SENSOR_COUNT - 1;

// Bits returned from readLineSensors() and readProximitySensors() to indicate which downward sensors detect the white line
// ( <g_blackThreshold) and which proximity sensors detect a nearby opponent (>=PROXIMITY_THRESHOLD).
const uint8_t LEFT_BIT = 1 << 1;
const uint8_t RIGHT_BIT = 1 << 0;
const uint8_t LEFT_AND_RIGHT_BITS = LEFT_BIT | RIGHT_BIT;

// Values from Proximity Sensor >= to this value will be considered the opponent.
const uint8_t PROXIMITY_THRESHOLD = 5;

// This special value means to lunge until the white ring border is encountered. It is just a large timeout.
const uint32_t LUNGE_TO_RING_EDGE = 0x7FFFFFFF;

// Only lunge for a short period of time after turning at the edge of the ring.
const uint32_t LUNGE_AWAY_FROM_EDGE = 200;

// How many milliseconds should the creepForward() operation take?
const uint32_t CREEP_FORWARD_TIME_MS = 10 * 20;

// How many milliseconds should the scanRight/Left() operations take?
const uint32_t SCAN_TIME_MS = 5 * 20;

// How many milliseconds should the spinRight/Left() operations take?
const uint32_t SPIN_TIME_MS = 11 * 20;

// How many milliseconds should the followRight/Left() operations take?
const uint32_t FOLLOW_TIME_MS = 20;

// How many milliseconds should the aboutFace() backup take?
const uint32_t ABOUT_FACE_BACKUP_TIME_MS = 10 * 20;

// How many milliseconds should the aboutFace() 180 turn take?
const uint32_t ABOUT_FACE_TURN_TIME_MS = 20 * 20;

// Charge melody played at beginning of match.
const char chargeMelody[] = "O4 T100 V15 L4 MS g12>c12>e12>G6>E12 ML>G2";

// Globals representing sensors and actuators on the Zumo 2040 Robot.
static Motors g_motors;
static LineSensors g_lineSensors;
static ProximitySensors g_proximitySensors;
static Buzzer g_buzzer;
static OLED g_display;
static RGBLEDs g_leds;
static ButtonA g_buttonA;

// Threshold to distinguish between black and white areas of the Sumo ring using the line sensors.
// Will be determined at init time by sampling the black surface underneath the bot when it first comes out of reset.
static uint16_t g_blackThreshold = 0;

// Last Proximity Sensor reading.
static uint8_t g_lastProximityBits = 0;


void setup() {
  // Make sure that all of the RGB LEDs are turned off after a reset.
  g_leds.setBrightness(0);

  // Sets black threshold to 1/4 the average of the far left and right line sensor readings.
  // Zumo robot must be placed over black playing surface before this code runs.
  g_lineSensors.read();
  uint16_t leftLineSensorValue = g_lineSensors.rawSensorValues[LEFT_SENSOR];
  uint16_t rightLineSensorValue = g_lineSensors.rawSensorValues[RIGHT_SENSOR];
  g_blackThreshold = (leftLineSensorValue + rightLineSensorValue) / (2 * 4);

  // Make sure that motors are in the stop state.
  g_motors.setSpeeds(MOTOR_STOP, MOTOR_STOP);

  // Prompt user to press the A button to start the 5 second count down.
  g_display.setLayout11x4();
  g_display.print("  Press A");
  g_display.gotoXY(0, 1);
  g_display.print("  To Start");
  g_buttonA.waitForPress();

  // 5 second count down with audio.
  g_display.setLayout8x2();
  uint32_t prevTimeLeft = 6;
  uint32_t startTime = millis();
  while (true) {
    uint32_t elapsedTime = millis() - startTime;
    if (elapsedTime >= 5000) {
      break;
    }
    uint32_t timeLeft = ((5000 - elapsedTime) + 999) / 1000;
    g_display.gotoXY(4, 0);
    g_display.print(timeLeft);
    if (timeLeft != prevTimeLeft) {
      g_buzzer.playNote(NOTE_G(3), 200, 15);
      prevTimeLeft = timeLeft;
    }
  }
  g_display.clear();
  g_display.print("   GO!");
  g_buzzer.play(chargeMelody);

  // Lunge forward towards the opponent and immediately try to push them out of the ring.
  lunge(LUNGE_TO_RING_EDGE);
}

// Lunge forward (hopefully towards the opponent) until reaching the white ring border.
// NOTE: I modified the original Parallax version:
//       * No longer a fixed timed, 200ms, event. Now take lungeTime_ms as a parameter.
//         * It will lunge until white ring edge is encountered when the opponent is detected.
//         * It will still just lunge for 200ms when moving away from the ring edge.
//       * No longer call matchOver() if the ring perimeter is encountered. It instead returns to the caller.
void lunge(uint32_t lungeTime_ms) {
  g_motors.setSpeeds(MOTOR_FWD_FAST, MOTOR_FWD_FAST);

  uint32_t startTime = millis();
  while (millis() - startTime < lungeTime_ms) {
    uint8_t lineBits = readLineSensors();
    if (lineBits != 0) {
      // At least one of the line sensors is detecting the white ring perimeter so return to caller.
      break;
    }
  }
}

uint8_t readLineSensors() {
  uint8_t sensorBits = 0;

  // Read the downward facing line sensors.
  g_lineSensors.read();
  uint16_t leftLineSensorValue = g_lineSensors.rawSensorValues[LEFT_SENSOR];
  uint16_t rightLineSensorValue = g_lineSensors.rawSensorValues[RIGHT_SENSOR];

  // Return with 2 bits.
  // Bit 0 for right sensor and bit 1 for left sensor.
  //   0 = black & 1 = white
  sensorBits |= leftLineSensorValue < g_blackThreshold ? LEFT_BIT : 0;
  sensorBits |= rightLineSensorValue < g_blackThreshold ? RIGHT_BIT : 0;

  return sensorBits;
}

void loop() {
  // If not on the Shikiri line (border), continue to look for opponent,
  // otherwise, spin back toward center and resume search.
  uint8_t lineBits = readLineSensors();
  switch (lineBits) {
    case 0:
      searchForOpponent();
      break;
    case RIGHT_BIT:
      spinLeft();
      break;
    case LEFT_BIT:
      spinRight();
      break;
    case LEFT_AND_RIGHT_BITS:
      aboutFace();
      break;
  }
}

void searchForOpponent() {
  // If opponent is not in view, scan last known direction. Turn toward
  // opponent if seen by one "eye" -- if both, lunge forward.
  uint8_t irBits = readProximitySensors();
  switch (irBits) {
    case 0:
      scan();
      break;
    case RIGHT_BIT:
      followRight(irBits);
      break;
    case LEFT_BIT:
      followLeft(irBits);
      break;
    case LEFT_AND_RIGHT_BITS:
      lunge(LUNGE_TO_RING_EDGE);
      break;
  }
}

uint8_t readProximitySensors() {
  uint8_t sensorBits = 0;

  // Read the IR proximity sensors.
  g_proximitySensors.read();

  // Return with 2 bits.
  // Bit 0 for right obstacle detection and bit 1 for left obstacle detection.
  //   0 = no obstacle/opponent detected.
  //   1 = obstacle/opponent detected.
  sensorBits |= g_proximitySensors.countsFrontWithLeftLeds() >= PROXIMITY_THRESHOLD ? LEFT_BIT : 0;
  sensorBits |= g_proximitySensors.countsFrontWithRightLeds() >= PROXIMITY_THRESHOLD ? RIGHT_BIT : 0;

  return sensorBits;
}

void scan() {
  // Continue scanning in the direction where opponent was last seen.
  switch (g_lastProximityBits) {
    case 0:
      moveForward();
      break;
    case RIGHT_BIT:
      scanRight();
      break;
    case LEFT_BIT:
      scanLeft();
      break;
    case LEFT_AND_RIGHT_BITS:
      // NOTE: Original BASIC Stamp 2 code had nothing for this condition.
      moveForward();
      break;
  }
}

void moveForward() {
  creepForward();
}

void creepForward() {
  g_motors.setSpeeds(MOTOR_FWD_SLOW, MOTOR_FWD_SLOW);
  delay(CREEP_FORWARD_TIME_MS);
}

void scanRight() {
  // Spin right, slowly.
  g_motors.setSpeeds(MOTOR_FWD_SLOW, MOTOR_REV_SLOW);
  delay(5 * 20);

  // Keep moving.
  creepForward();
}

void scanLeft() {
  // Spin left, slowly.
  g_motors.setSpeeds(MOTOR_REV_SLOW, MOTOR_FWD_SLOW);
  delay(5 * 20);

  // Keep moving.
  creepForward();
}

void followRight(uint8_t irBits) {
  // Spin right, fast.
  g_motors.setSpeeds(MOTOR_FWD_FAST, MOTOR_REV_FAST);
  delay(FOLLOW_TIME_MS);

  // Save last direction to opponent found.
  g_lastProximityBits = irBits;
}

void followLeft(uint8_t irBits) {
  // Spin left, fast.
  g_motors.setSpeeds(MOTOR_REV_FAST, MOTOR_FWD_FAST);
  delay(FOLLOW_TIME_MS);

  // Save last direction to opponent found.
  g_lastProximityBits = irBits;
}

void spinLeft() {
  // Right downward line sensor was active so turn left, fast.
  g_motors.setSpeeds(MOTOR_REV_FAST, MOTOR_FWD_FAST);
  delay(SPIN_TIME_MS);

  // Clear scan direction.
  g_lastProximityBits = 0;
  lunge(LUNGE_AWAY_FROM_EDGE);
}

void spinRight() {
  // Left downward line sensor was active so turn right, fast.
  g_motors.setSpeeds(MOTOR_FWD_FAST, MOTOR_REV_FAST);
  delay(SPIN_TIME_MS);

  // Clear scan direction.
  g_lastProximityBits = 0;
  lunge(LUNGE_AWAY_FROM_EDGE);
}

void aboutFace() {
  // Both line sensors on Shikiri.
  // Back up from edge.
  g_motors.setSpeeds(MOTOR_REV_FAST, MOTOR_REV_FAST);
  delay(ABOUT_FACE_BACKUP_TIME_MS);

  // Turn around to the right, quickly.
  g_motors.setSpeeds(MOTOR_FWD_FAST, MOTOR_REV_FAST);
  delay(ABOUT_FACE_TURN_TIME_MS);

  // Clear scan direction.
  g_lastProximityBits = 0;
  lunge(LUNGE_AWAY_FROM_EDGE);
}
