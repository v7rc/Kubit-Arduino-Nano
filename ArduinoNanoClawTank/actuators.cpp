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
  // Enter the safe H-bridge state (DIR=LOW, PWM=LOW) before changing mode.
  analogWrite(pwmPin, 0);
  digitalWrite(directionPin, LOW);
  if (command.stopped) {
    return;
  }

  digitalWrite(directionPin, command.directionHigh ? HIGH : LOW);
  analogWrite(pwmPin, command.pwmOutput);
}
}  // namespace

void actuatorsBegin() {
  // Preload the safe H-bridge state before enabling output mode.
  digitalWrite(Config::M1_PWM_PIN, LOW);
  digitalWrite(Config::M2_PWM_PIN, LOW);
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

AppliedVehicleState applyPacket(const V7RCPacket& packet) {
  const TankDriveMix drive =
      mixTankDrive(packet.channels[0], packet.channels[1],
                   Config::MOTOR_DEADBAND, Config::STEERING_REVERSED);
  const uint16_t clawUs = constrainPulse(
      packet.channels[2], Config::CLAW_MIN_US, Config::CLAW_MAX_US);
  const uint16_t liftUs = constrainPulse(
      packet.channels[3], Config::LIFT_MIN_US, Config::LIFT_MAX_US);

  applyMotor(Config::M1_DIRECTION_PIN, Config::M1_PWM_PIN,
             makeMotorCommand(drive.m1, Config::M1_REVERSED));
  applyMotor(Config::M2_DIRECTION_PIN, Config::M2_PWM_PIN,
             makeMotorCommand(drive.m2, Config::M2_REVERSED));
  clawServo.writeMicroseconds(clawUs);
  liftServo.writeMicroseconds(liftUs);

  AppliedVehicleState state = {drive.m1, drive.m2, clawUs, liftUs};
  return state;
}

void stopMotors() {
  analogWrite(Config::M1_PWM_PIN, 0);
  analogWrite(Config::M2_PWM_PIN, 0);
  digitalWrite(Config::M1_DIRECTION_PIN, LOW);
  digitalWrite(Config::M2_DIRECTION_PIN, LOW);
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
