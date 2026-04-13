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
const float SENSOR_BOTTOM_READING_IN = 22.5;

// Sensor reading when the ball is at the very top of the tube
const float SENSOR_TOP_READING_IN = 2.0;

// Actual physical height of the tube
const float REAL_TUBE_HEIGHT_IN = 35.0;


// PWM output mapping for sensor
const uint8_t PWM_AT_TOP = 255;
const uint8_t PWM_AT_BOTTOM = 127;

void setup() {
  Serial.begin(115200);

  pinMode(FAN_PWM, OUTPUT);  // D9 = OC1A (fan control PWM)
  pinMode(TOF_PWM, OUTPUT);  // D5 PWM output for distance signal

  // ---- Timer1 configuration: 25 kHz PWM on D9 (OC1A) ----
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  ICR1 = 639;  // TOP value -> 25 kHz PWM frequency
  OCR1A = 0;    // Start with 0% duty cycle

  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

  // ---- Initialize the ToF sensor ----
  if (!lox.begin()) {
    Serial.println("Failed to boot VL53L0X");
    while (1)
      ;
  }

  Serial.println("VL53L0X Ready");

  OCR1A = ICR1;     // kick start fan
  delay (500);
}

void loop() {
  // -----------------------------
  // Existing fan control (unchanged)
  // -----------------------------
  int adc = analogRead(ctrlSigPin);

  float dutyPercent = 59.0 + (15.0 * adc / 1023.0);  // 0V->57%, 2.5V->67%, 5V->77%
  uint16_t ocr = (uint32_t)(dutyPercent * (ICR1 + 1) / 100.0);

  if (ocr > ICR1) ocr = ICR1;
  OCR1A = ocr;

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
      distance_in = 0.0;
    }

    // -----------------------------
    // BOTTOM saturation
    // If the ball is near the bottom, force distance to 35 inches
    // -----------------------------
    else if (raw_distance_in >= 22.5) {
      distance_in = REAL_TUBE_HEIGHT_IN;
    }

    // -----------------------------
    // Linear remapping
    // 2 in  -> 0 in
    // 18 in -> 35 in
    // -----------------------------
    else {
      distance_in =
        (raw_distance_in - SENSOR_TOP_READING_IN) * (REAL_TUBE_HEIGHT_IN / (SENSOR_BOTTOM_READING_IN - SENSOR_TOP_READING_IN));

      if (distance_in < 0.0) distance_in = 0.0;
      if (distance_in > REAL_TUBE_HEIGHT_IN) distance_in = REAL_TUBE_HEIGHT_IN;
    }

  } else {
    // If the sensor reading is invalid, set distance to zero
    raw_distance_in = 0.0;
    distance_in = 0.0;
  }

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
  Serial.print("A0 adc=");
  Serial.print(adc);

  Serial.print("  Fan OCR1A=");
  Serial.print(OCR1A);

  Serial.print("  RawDist(in)=");
  Serial.print(raw_distance_in);

  Serial.print("  MappedDist(in)=");
  Serial.print(distance_in);

  Serial.print("  PWM5=");
  Serial.println(pwmValue);

  delay(50);
}