#ifndef ARDUINO_NANO_CLAW_TANK_ACTUATORS_H
#define ARDUINO_NANO_CLAW_TANK_ACTUATORS_H

#include <Arduino.h>

#include "v7rc_protocol.h"

void actuatorsBegin();
void applyPacket(const V7RCPacket& packet);
void stopMotors();
void startBuzzer(unsigned long now, unsigned long durationMs);
void updateBuzzer(unsigned long now);

#endif
