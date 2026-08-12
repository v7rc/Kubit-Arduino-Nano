#include <Arduino.h>

#include "actuators.h"
#include "config.h"
#include "v7rc_protocol.h"

V7RCProtocol protocol;

bool hasReceivedValidPacket = false;
bool failsafeActive = true;
unsigned long lastValidPacketAt = 0;

void handleV7RCInput(unsigned long now) {
  while (Serial.available() > 0) {
    V7RCPacket packet;
    const uint8_t byte = static_cast<uint8_t>(Serial.read());
    if (protocol.feed(byte, packet)) {
      applyPacket(packet);
      lastValidPacketAt = now;
      hasReceivedValidPacket = true;
      failsafeActive = false;
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
  updateBuzzer(now);
}
