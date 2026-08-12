#include <Arduino.h>

namespace {
constexpr uint8_t LEFT_DIR_PIN = 4;
constexpr uint8_t LEFT_PWM_PIN = 5;
constexpr uint8_t RIGHT_PWM_PIN = 6;
constexpr uint8_t RIGHT_DIR_PIN = 7;
constexpr unsigned long UART_BAUD = 9600;
constexpr unsigned long PULSE_MS = 250;

bool pulseActive = false;
unsigned long pulseStartedAt = 0;

void setRightInputs(bool dirHigh, bool pwmHigh) {
  digitalWrite(RIGHT_DIR_PIN, dirHigh ? HIGH : LOW);
  digitalWrite(RIGHT_PWM_PIN, pwmHigh ? HIGH : LOW);
}

void stopAll() {
  digitalWrite(LEFT_DIR_PIN, LOW);
  digitalWrite(LEFT_PWM_PIN, LOW);
  setRightInputs(false, false);
}

void startTest(uint8_t state) {
  const bool dirHigh = (state & 0x02U) != 0;
  const bool pwmHigh = (state & 0x01U) != 0;
  stopAll();
  setRightInputs(dirHigh, pwmHigh);
  pulseStartedAt = millis();
  pulseActive = true;

  Serial.print(F("RIGHT DIR="));
  Serial.print(dirHigh ? 1 : 0);
  Serial.print(F(" PWM="));
  Serial.println(pwmHigh ? 1 : 0);
}
}  // namespace

void setup() {
  digitalWrite(LEFT_DIR_PIN, LOW);
  digitalWrite(LEFT_PWM_PIN, LOW);
  digitalWrite(RIGHT_DIR_PIN, LOW);
  digitalWrite(RIGHT_PWM_PIN, LOW);
  pinMode(LEFT_DIR_PIN, OUTPUT);
  pinMode(LEFT_PWM_PIN, OUTPUT);
  pinMode(RIGHT_DIR_PIN, OUTPUT);
  pinMode(RIGHT_PWM_PIN, OUTPUT);
  stopAll();

  Serial.begin(UART_BAUD);
  Serial.println(F("RIGHT MOTOR TRUTH TABLE READY: send 0,1,2,3"));
}

void loop() {
  if (Serial.available() > 0) {
    const int input = Serial.read();
    if (input >= '0' && input <= '3') {
      startTest(static_cast<uint8_t>(input - '0'));
    }
  }

  if (pulseActive &&
      static_cast<unsigned long>(millis() - pulseStartedAt) >= PULSE_MS) {
    stopAll();
    pulseActive = false;
    Serial.println(F("RIGHT SAFE DIR=0 PWM=0"));
  }
}
