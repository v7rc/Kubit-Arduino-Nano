#include <Arduino.h>
#include <SoftwareSerial.h>

#include "actuators.h"
#include "config.h"
#include "v7rc_protocol.h"

SoftwareSerial bluetoothSerial(Config::BT_RX_PIN, Config::BT_TX_PIN);
V7RCProtocol protocol;

bool hasReceivedValidPacket = false;
bool failsafeActive = true;
unsigned long lastValidPacketAt = 0;

void handleBluetoothInput(unsigned long now) {
  while (bluetoothSerial.available() > 0) {
    V7RCPacket packet;
    const uint8_t byte = static_cast<uint8_t>(bluetoothSerial.read());
    if (protocol.feed(byte, packet)) {
      applyPacket(packet);
      lastValidPacketAt = now;
      hasReceivedValidPacket = true;
      failsafeActive = false;

      if (Config::DEBUG_ENABLED) {
        Serial.print(F("V7RC:"));
        for (uint8_t channel = 0; channel < 4; ++channel) {
          Serial.print(' ');
          Serial.print(packet.channels[channel]);
        }
        Serial.println();
      }
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

    if (Config::DEBUG_ENABLED) {
      Serial.println(F("FAILSAFE: motors stopped"));
    }
  }
}

void setup() {
  // Safety-critical initialization must remain the first operation.
  actuatorsBegin();

  if (Config::DEBUG_ENABLED) {
    Serial.begin(Config::DEBUG_BAUD);
    Serial.println(F("Arduino Nano Claw Tank ready"));
  }

  bluetoothSerial.begin(Config::BT_BAUD);
  startBuzzer(millis(), Config::STARTUP_BEEP_MS);
}

void loop() {
  const unsigned long now = millis();
  handleBluetoothInput(now);
  updateFailsafe(now);
  updateBuzzer(now);
}
