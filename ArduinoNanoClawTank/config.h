#ifndef ARDUINO_NANO_CLAW_TANK_CONFIG_H
#define ARDUINO_NANO_CLAW_TANK_CONFIG_H

#include <Arduino.h>

namespace Config {

// V7RC receiver uses the Nano hardware UART input on D0/RX. D1/TX is not
// connected and the firmware never writes to Serial.
constexpr unsigned long V7RC_UART_BAUD = 9600;

constexpr uint8_t M1_DIRECTION_PIN = 4;
constexpr uint8_t M1_PWM_PIN = 5;
constexpr uint8_t M2_PWM_PIN = 6;
constexpr uint8_t M2_DIRECTION_PIN = 7;
constexpr bool M1_REVERSED = false;
constexpr bool M2_REVERSED = false;
// With false: CH1=1000 turns left and CH1=2000 turns right. Toggle this if
// the installed drivetrain responds in the opposite direction.
constexpr bool STEERING_REVERSED = false;

constexpr uint8_t BUZZER_PIN = 8;
// Leave false until the buzzer is confirmed to be an active buzzer.
constexpr bool BUZZER_ENABLED = false;

constexpr uint8_t CLAW_SERVO_PIN = 9;
constexpr uint8_t LIFT_SERVO_PIN = 10;
constexpr uint8_t AUX_SERVO_PIN = 11;

constexpr uint16_t CHANNEL_MIN = 1000;
constexpr uint16_t CHANNEL_CENTER = 1500;
constexpr uint16_t CHANNEL_MAX = 2000;
constexpr uint16_t MOTOR_DEADBAND = 25;

constexpr uint16_t CLAW_MIN_US = 1000;
constexpr uint16_t CLAW_CENTER_US = 1500;
constexpr uint16_t CLAW_MAX_US = 2000;
constexpr uint16_t LIFT_MIN_US = 1000;
constexpr uint16_t LIFT_CENTER_US = 1500;
constexpr uint16_t LIFT_MAX_US = 2000;
constexpr uint16_t AUX_MIN_US = 1000;
constexpr uint16_t AUX_CENTER_US = 1500;
constexpr uint16_t AUX_MAX_US = 2000;

constexpr unsigned long FAILSAFE_TIMEOUT_MS = 300;
constexpr unsigned long STARTUP_BEEP_MS = 80;
constexpr unsigned long FAILSAFE_BEEP_MS = 120;

}  // namespace Config

#endif
