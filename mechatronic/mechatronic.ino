#include <Wire.h>
#include <VL53L1X.h>

#define FAN_PWM 9
#define TOF_PWM 5

// ROI size (SPADs): 4 is the minimum, 16 is full FOV.
// Smaller = narrower field of view = less acrylic tube wall in the measurement.
// Start at 4 and increase if you get frequent signal-fail readings.
#define ROI_SIZE 4

VL53L1X sensor;

const int ctrlSigPin = A0;

const uint8_t PWM_AT_TOP    = 255;
const uint8_t PWM_AT_BOTTOM = 127;
const float   MAX_RANGE_MM  = 889.0;  // 35 in -- physical tube height

int16_t distance_mm = 0;

void busRecover() {
  pinMode(SCL, OUTPUT);
  pinMode(SDA, OUTPUT);
  digitalWrite(SDA, HIGH);
  for (int i = 0; i < 9; i++) {
    digitalWrite(SCL, HIGH); delayMicroseconds(5);
    digitalWrite(SCL, LOW);  delayMicroseconds(5);
  }
  digitalWrite(SDA, LOW);  delayMicroseconds(5);
  digitalWrite(SCL, HIGH); delayMicroseconds(5);
  digitalWrite(SDA, HIGH); delayMicroseconds(5);
  TWCR = 0;
  pinMode(SCL, INPUT);
  pinMode(SDA, INPUT);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Serial OK");

  pinMode(FAN_PWM, OUTPUT);
  pinMode(TOF_PWM, OUTPUT);

  // Timer1: 25 kHz PWM on D9 (OC1A)
  TCCR1A = 0; TCCR1B = 0; TCNT1 = 0;
  ICR1   = 639;
  OCR1A  = 0;
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13)  | (1 << WGM12) | (1 << CS10);

  busRecover();
  Wire.begin();
  Wire.setClock(400000);
  delay(100);

  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Failed to init VL53L1X");
    while (1) delay(10);
  }
  Serial.println("VL53L1X ready");

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  // Narrow the ROI to reduce tube-wall reflections.
  sensor.setROISize(ROI_SIZE, ROI_SIZE);
  sensor.startContinuous(50);

  delay(100);
}

void loop() {
  // Fan control -- direct map: 0V -> off, 5V -> full speed
  int adc = analogRead(ctrlSigPin);
  uint16_t ocr = (uint32_t)adc * ICR1 / 1023;
  OCR1A = ocr;

  // Distance
  uint16_t reading = sensor.read(false);

  if (sensor.timeoutOccurred()) {
    Serial.println("TIMEOUT");
  } else if (sensor.ranging_data.range_status == VL53L1X::RangeValid) {
    distance_mm = (int16_t)reading;
  }

  // Map raw distance directly to PWM -- no remapping needed
  // 0 mm (top)       -> PWM 255
  // 889 mm (bottom)  -> PWM 127
  float frac = (float)distance_mm / MAX_RANGE_MM;
  if (frac < 0.0) frac = 0.0;
  if (frac > 1.0) frac = 1.0;
  uint8_t pwmValue = (uint8_t)(PWM_AT_TOP - frac * (PWM_AT_TOP - PWM_AT_BOTTOM));
  analogWrite(TOF_PWM, pwmValue);

  Serial.print("adc=");      Serial.print(adc);
  Serial.print("  OCR1A=");  Serial.print(OCR1A);
  Serial.print("  status="); Serial.print((int)sensor.ranging_data.range_status);
  Serial.print("  mm=");     Serial.print(distance_mm);
  Serial.print("  in=");     Serial.print((float)distance_mm / 25.4, 1);
  Serial.print("  PWM5=");   Serial.println(pwmValue);
}
