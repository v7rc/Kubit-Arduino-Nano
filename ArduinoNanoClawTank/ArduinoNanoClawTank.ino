#include <Arduino.h>

#include "actuators.h"
#include "config.h"
#include "v7rc_protocol.h"

V7RCProtocol protocol;

bool hasReceivedValidPacket = false;
bool failsafeActive = true;
unsigned long lastValidPacketAt = 0;
unsigned long lastDebugOutputAt = 0;

struct DebugSnapshot {
  AppliedVehicleState vehicle;
  bool failsafe;
};

DebugSnapshot pendingDebugSnapshot = {{0, 0, Config::CLAW_CENTER_US,
                                       Config::LIFT_CENTER_US},
                                      true};
DebugSnapshot lastDebugSnapshot = pendingDebugSnapshot;
AppliedVehicleState currentVehicleState = pendingDebugSnapshot.vehicle;
bool debugOutputPending = false;
bool hasDebugOutput = false;

bool debugSnapshotsEqual(const DebugSnapshot& left,
                         const DebugSnapshot& right) {
  return left.vehicle.m1Demand == right.vehicle.m1Demand &&
         left.vehicle.m2Demand == right.vehicle.m2Demand &&
         left.vehicle.clawUs == right.vehicle.clawUs &&
         left.vehicle.liftUs == right.vehicle.liftUs &&
         left.failsafe == right.failsafe;
}

void queueDebugOutput(const AppliedVehicleState& vehicle, bool failsafe) {
  if (!Config::DEBUG_OUTPUT_ENABLED) {
    return;
  }

  DebugSnapshot next = {vehicle, failsafe};
  if (!hasDebugOutput || !debugSnapshotsEqual(next, lastDebugSnapshot)) {
    pendingDebugSnapshot = next;
    debugOutputPending = true;
  } else {
    // Cancel an older pending state if the vehicle returned to the state that
    // was already reported before the rate-limit interval elapsed.
    debugOutputPending = false;
  }
}

void updateDebugOutput(unsigned long now) {
  if (!debugOutputPending ||
      static_cast<unsigned long>(now - lastDebugOutputAt) <
          Config::DEBUG_OUTPUT_INTERVAL_MS ||
      Serial.availableForWrite() < Config::DEBUG_MIN_WRITE_SPACE) {
    return;
  }

  Serial.print(F("V M1="));
  Serial.print(pendingDebugSnapshot.vehicle.m1Demand);
  Serial.print(F(" M2="));
  Serial.print(pendingDebugSnapshot.vehicle.m2Demand);
  Serial.print(F(" S1="));
  Serial.print(pendingDebugSnapshot.vehicle.clawUs);
  Serial.print(F(" S2="));
  Serial.print(pendingDebugSnapshot.vehicle.liftUs);
  Serial.print(F(" FS="));
  Serial.println(pendingDebugSnapshot.failsafe ? 1 : 0);

  lastDebugSnapshot = pendingDebugSnapshot;
  hasDebugOutput = true;
  debugOutputPending = false;
  lastDebugOutputAt = now;
}

void handleV7RCInput(unsigned long now) {
  while (Serial.available() > 0) {
    V7RCPacket packet;
    const uint8_t byte = static_cast<uint8_t>(Serial.read());
    if (protocol.feed(byte, packet)) {
      currentVehicleState = applyPacket(packet);
      lastValidPacketAt = now;
      hasReceivedValidPacket = true;
      failsafeActive = false;
      queueDebugOutput(currentVehicleState, false);
    }
  }
}

void updateFailsafe(unsigned long now) {
  if (!hasReceivedValidPacket || failsafeActive) {
    return;
  }

  if (static_cast<unsigned long>(now - lastValidPacketAt) >
      Config::FAILSAFE_TIMEOUT_MS) {
    stopMotors();
    failsafeActive = true;
    currentVehicleState.m1Demand = 0;
    currentVehicleState.m2Demand = 0;
    queueDebugOutput(currentVehicleState, true);
    startBuzzer(now, Config::FAILSAFE_BEEP_MS);
  }
}

void setup() {
  // Safety-critical initialization must remain the first operation.
  actuatorsBegin();
  Serial.begin(Config::V7RC_UART_BAUD);
  startBuzzer(millis(), Config::STARTUP_BEEP_MS);
}

void loop() {
  const unsigned long now = millis();
  handleV7RCInput(now);
  updateFailsafe(now);
  updateDebugOutput(now);
  updateBuzzer(now);
}
