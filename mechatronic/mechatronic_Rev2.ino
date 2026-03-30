#include <Wire.h>
#include "Adafruit_VL53L0X.h"

// Sensor reading when the ball is at the very bottom of the tube
// (Due to tube reflections, the sensor reads ~18 in instead of the actual 35 in)

// Linear remapping converts the compressed sensor range (2–18 in)
// into the actual physical height range (0–35 in)

#define FAN_PWM 9
#define TOF_PWM 5  // PWM output used to represent ToF distance

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int ctrlSigPin = A0;

// -----------------------------
// Calibration values
// -----------------------------
// Sensor reading when the ball is at the very bottom of the tube
const float SENSOR_BOTTOM_READING_IN = 23;

// Sensor reading when the ball is at the very top of the tube
const float SENSOR_TOP_READING_IN = 2.5;

// Actual physical height of the tube
const float REAL_TUBE_HEIGHT_IN = 27.0;


// PWM output mapping for sensor
const uint8_t PWM_AT_TOP = 255; 
const uint8_t PWM_AT_BOTTOM = 127;


// -----------------------------
// Bottom-mode tuning
// -----------------------------
const float BOTTOM_ENTER_RAW = 20.8;   // enter bottom mode when raw reading gets this large
const float BOTTOM_EXIT_RAW  = 19.5;   // leave bottom mode only after ball clearly rises

const float BOTTOM_KICK_DUTY = 61.5;   // short boost
const float BOTTOM_HOLD_DUTY = 60.7;   // hold near desired bottom

const unsigned long BOTTOM_KICK_TIME_MS = 300;  // kick duration

bool inBottomMode = false;
bool bottomKickActive = false;
unsigned long bottomKickStartMs = 0;


void setup() {
  Serial.begin(115200);

  pinMode(FAN_PWM, OUTPUT);  // D9 = OC1A (fan control PWM)
  pinMode(TOF_PWM, OUTPUT);  // D5 PWM output for distance signal

  // ---- Timer1 configuration: 25 kHz PWM on D9 (OC1A) ----
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  ICR1 = 1279;  // TOP value -> 25 kHz PWM frequency
  OCR1A = 0;    // Start with 0% duty cycle

  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

  // ---- Initialize the ToF sensor ----
  if (!lox.begin()) {
    Serial.println("Failed to boot VL53L0X");
    while (1);
  }

  Serial.println("VL53L0X Ready");
}

void loop() {
  // -----------------------------
  // Existing fan control (unchanged)
  // -----------------------------
  int adc = analogRead(ctrlSigPin);

  float dutyPercent = 62.0 + (10.0 * adc / 1023.0);  // 0V->57%, 2.5V->67%, 5V->77%
  // uint16_t ocr = (uint32_t)(dutyPercent * (ICR1 + 1) / 100.0);

  // if (ocr > ICR1) ocr = ICR1;
  // OCR1A = ocr;

  // -----------------------------
  // ToF distance measurement -> PWM output on D5
  // -----------------------------
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  float raw_distance_in = 0.0;
  float distance_in = 0.0;

  if (measure.RangeStatus != 4) {
    float distance_mm = measure.RangeMilliMeter;
    raw_distance_in = distance_mm / 25.4;

    // Limit the raw distance to the valid sensor range
    if (raw_distance_in < 0.0) raw_distance_in = 0.0;
    if (raw_distance_in > SENSOR_BOTTOM_READING_IN) {
      raw_distance_in = SENSOR_BOTTOM_READING_IN;
    }

    // -----------------------------
    // TOP saturation
    // If the ball is near the top, force distance to 0 inches
    // -----------------------------
    if (raw_distance_in <= SENSOR_TOP_READING_IN) {
      distance_in = 0;
    }

    // -----------------------------
    // BOTTOM saturation
    // If the ball is near the bottom, force distance to 35 inches
    // -----------------------------
    else if (raw_distance_in >= SENSOR_BOTTOM_READING_IN) {
      distance_in = REAL_TUBE_HEIGHT_IN;
    }

    // -----------------------------
    // Linear remapping
    // 2 in  -> 0 in
    // 18 in -> 35 in
    // -----------------------------
    else {
      distance_in =
        (raw_distance_in - SENSOR_TOP_READING_IN) *
        (REAL_TUBE_HEIGHT_IN / (SENSOR_BOTTOM_READING_IN - SENSOR_TOP_READING_IN));

      if (distance_in < 0.0) distance_in = 0.0;
      if (distance_in > REAL_TUBE_HEIGHT_IN) distance_in = REAL_TUBE_HEIGHT_IN;
    }

  } else {
    // If the sensor reading is invalid, set distance to zero
    //raw_distance_in = 0.0;
    distance_in = 0.0;
  }
  // -----------------------------
  // Bottom mode state machine
  // -----------------------------
  if (!inBottomMode && raw_distance_in >= BOTTOM_ENTER_RAW) {
    inBottomMode = true;
    bottomKickActive = true;
    bottomKickStartMs = millis();
  }

  if (inBottomMode && raw_distance_in <= BOTTOM_EXIT_RAW) {
    inBottomMode = false;
    bottomKickActive = false;
  }

  if (bottomKickActive && (millis() - bottomKickStartMs >= BOTTOM_KICK_TIME_MS)) {
    bottomKickActive = false;
  }

  // -----------------------------
  // Override duty in bottom mode
  // -----------------------------
  if (inBottomMode) {
    if (bottomKickActive) {
      dutyPercent = BOTTOM_KICK_DUTY;
    } else {
      dutyPercent = BOTTOM_HOLD_DUTY;
    }
  }

  // Safety clamp
  if (dutyPercent < 0.0) dutyPercent = 0.0;
  if (dutyPercent > 100.0) dutyPercent = 100.0;

  uint16_t ocr = (uint16_t)(dutyPercent * (ICR1 + 1) / 100.0 + 0.5);
  if (ocr > ICR1) ocr = ICR1;
  OCR1A = ocr;
  // -----------------------------
  // Convert distance to PWM (0–255)
  // 35 inches -> PWM = 127 (get 12 volts out)
  // 0 inches  -> PWM = 255
  // -----------------------------
  float frac = distance_in / REAL_TUBE_HEIGHT_IN;
  uint8_t pwmValue = (uint8_t)(PWM_AT_TOP - frac * (PWM_AT_TOP - PWM_AT_BOTTOM));

  analogWrite(TOF_PWM, pwmValue);

  // -----------------------------
  // Serial debug output
  // -----------------------------
  // Serial.print("A0 adc=");
  // Serial.print(adc);

  // Serial.print("  Fan OCR1A=");
  // float fanPWMVal = (OCR1A / 1023) * 100;
  // Serial.print((OCR1A / 1023) * 100);

  // Serial.print("  RawDist(in)=");
  // Serial.print(raw_distance_in);

  // Serial.print("  MappedDist(in)=");
  // Serial.print(distance_in);

  // Serial.print("  PWM5=");
  // Serial.println(pwmValue);
  Serial.print(adc);
  Serial.print(" ");
  Serial.print(raw_distance_in);
  Serial.print(" ");
  Serial.println(distance_in);
  delay(50);
}