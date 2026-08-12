#include "actuators.h"

#include <Servo.h>

#include "config.h"
#include "control_math.h"

namespace {
Servo clawServo;
Servo liftServo;
Servo auxServo;
bool buzzerActive = false;
unsigned long buzzerStartedAt = 0;
unsigned long buzzerDuration = 0;

uint16_t constrainPulse(uint16_t value, uint16_t minimum, uint16_t maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

void applyMotor(uint8_t directionPin, uint8_t pwmPin,
                const MotorCommand& command) {
  // PWM is cleared before changing direction to reduce abrupt transients.
  analogWrite(pwmPin, 0);
  digitalWrite(directionPin, command.directionHigh ? HIGH : LOW);
  analogWrite(pwmPin, command.pwm);
}
}  // namespace

void actuatorsBegin() {
  pinMode(Config::M1_PWM_PIN, OUTPUT);
  pinMode(Config::M2_PWM_PIN, OUTPUT);
  analogWrite(Config::M1_PWM_PIN, 0);
  analogWrite(Config::M2_PWM_PIN, 0);

  pinMode(Config::M1_DIRECTION_PIN, OUTPUT);
  pinMode(Config::M2_DIRECTION_PIN, OUTPUT);
  digitalWrite(Config::M1_DIRECTION_PIN, LOW);
  digitalWrite(Config::M2_DIRECTION_PIN, LOW);

  pinMode(Config::BUZZER_PIN, OUTPUT);
  digitalWrite(Config::BUZZER_PIN, LOW);

  clawServo.attach(Config::CLAW_SERVO_PIN, Config::CLAW_MIN_US,
                   Config::CLAW_MAX_US);
  liftServo.attach(Config::LIFT_SERVO_PIN, Config::LIFT_MIN_US,
                   Config::LIFT_MAX_US);
  auxServo.attach(Config::AUX_SERVO_PIN, Config::AUX_MIN_US,
                  Config::AUX_MAX_US);
  clawServo.writeMicroseconds(Config::CLAW_CENTER_US);
  liftServo.writeMicroseconds(Config::LIFT_CENTER_US);
  auxServo.writeMicroseconds(Config::AUX_CENTER_US);
}

void applyPacket(const V7RCPacket& packet) {
  applyMotor(Config::M1_DIRECTION_PIN, Config::M1_PWM_PIN,
             makeMotorCommand(packet.channels[0], Config::M1_REVERSED,
                              Config::MOTOR_DEADBAND));
  applyMotor(Config::M2_DIRECTION_PIN, Config::M2_PWM_PIN,
             makeMotorCommand(packet.channels[1], Config::M2_REVERSED,
                              Config::MOTOR_DEADBAND));

  clawServo.writeMicroseconds(constrainPulse(
      packet.channels[2], Config::CLAW_MIN_US, Config::CLAW_MAX_US));
  liftServo.writeMicroseconds(constrainPulse(
      packet.channels[3], Config::LIFT_MIN_US, Config::LIFT_MAX_US));
}

void stopMotors() {
  analogWrite(Config::M1_PWM_PIN, 0);
  analogWrite(Config::M2_PWM_PIN, 0);
}

void startBuzzer(unsigned long now, unsigned long durationMs) {
  if (!Config::BUZZER_ENABLED) {
    return;
  }
  buzzerStartedAt = now;
  buzzerDuration = durationMs;
  buzzerActive = true;
  digitalWrite(Config::BUZZER_PIN, HIGH);
}

void updateBuzzer(unsigned long now) {
  if (buzzerActive &&
      static_cast<unsigned long>(now - buzzerStartedAt) >= buzzerDuration) {
    buzzerActive = false;
    digitalWrite(Config::BUZZER_PIN, LOW);
  }
}
