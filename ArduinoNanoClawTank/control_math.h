#ifndef ARDUINO_NANO_CLAW_TANK_CONTROL_MATH_H
#define ARDUINO_NANO_CLAW_TANK_CONTROL_MATH_H

#include <stdint.h>

struct MotorCommand {
  uint8_t pwm;
  bool directionHigh;
};

struct TankDriveMix {
  // Signed motor demand: -500 is full reverse, 0 stop, +500 full forward.
  int16_t m1;
  int16_t m2;
};

TankDriveMix mixTankDrive(uint16_t steeringChannel, uint16_t throttleChannel,
                          uint16_t deadband, bool steeringReversed);
MotorCommand makeMotorCommand(int16_t motorDemand, bool reversed);

#endif
