#ifndef ARDUINO_NANO_CLAW_TANK_CONTROL_MATH_H
#define ARDUINO_NANO_CLAW_TANK_CONTROL_MATH_H

#include <stdint.h>

struct MotorCommand {
  uint8_t pwm;
  bool directionHigh;
};

MotorCommand makeMotorCommand(uint16_t channelValue, bool reversed,
                              uint16_t deadband);

#endif
