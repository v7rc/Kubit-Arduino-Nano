#ifndef ARDUINO_NANO_CLAW_TANK_V7RC_PROTOCOL_H
#define ARDUINO_NANO_CLAW_TANK_V7RC_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

struct V7RCPacket {
  uint16_t channels[4];
};

class V7RCProtocol {
 public:
  V7RCProtocol();

  // Returns true only when byte completes a valid packet. Invalid input never
  // changes output and the parser automatically searches for the next "SRT".
  bool feed(uint8_t byte, V7RCPacket& output);
  void reset();

 private:
  static const uint8_t PACKET_LENGTH = 20;
  char buffer_[PACKET_LENGTH];
  uint8_t index_;

  bool isExpectedByte(uint8_t byte) const;
  bool decode(V7RCPacket& output) const;
  void restartFrom(uint8_t byte);
};

#endif
