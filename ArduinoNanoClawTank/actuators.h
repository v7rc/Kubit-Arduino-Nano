#ifndef ARDUINO_NANO_CLAW_TANK_ACTUATORS_H
#define ARDUINO_NANO_CLAW_TANK_ACTUATORS_H

#include <Arduino.h>

#include "v7rc_protocol.h"

struct AppliedVehicleState {
  int16_t m1Demand;
  int16_t m2Demand;
  uint16_t clawUs;
  uint16_t liftUs;
};

void actuatorsBegin();
AppliedVehicleState applyPacket(const V7RCPacket& packet);
void stopMotors();
void startBuzzer(unsigned long now, unsigned long durationMs);
void updateBuzzer(unsigned long now);

#endif
