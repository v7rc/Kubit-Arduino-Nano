#include "v7rc_protocol.h"

namespace {
constexpr uint16_t kMinimumChannel = 1000;
constexpr uint16_t kMaximumChannel = 2000;
}

V7RCProtocol::V7RCProtocol() : index_(0) {}

void V7RCProtocol::reset() {
  index_ = 0;
}

bool V7RCProtocol::feed(uint8_t byte, V7RCPacket& output) {
  if (index_ == 0) {
    if (byte == 'S') {
      buffer_[0] = static_cast<char>(byte);
      index_ = 1;
    }
    return false;
  }

  if (!isExpectedByte(byte)) {
    restartFrom(byte);
    return false;
  }

  buffer_[index_++] = static_cast<char>(byte);
  if (index_ < PACKET_LENGTH) {
    return false;
  }

  index_ = 0;
  return decode(output);
}

bool V7RCProtocol::isExpectedByte(uint8_t byte) const {
  if (index_ == 1) {
    return byte == 'R';
  }
  if (index_ == 2) {
    return byte == 'T';
  }
  if (index_ >= 3 && index_ <= 18) {
    return byte >= '0' && byte <= '9';
  }
  return index_ == 19 && byte == '#';
}

bool V7RCProtocol::decode(V7RCPacket& output) const {
  V7RCPacket candidate = {{0, 0, 0, 0}};

  for (uint8_t channel = 0; channel < 4; ++channel) {
    const uint8_t offset = 3 + channel * 4;
    uint16_t value = 0;
    for (uint8_t digit = 0; digit < 4; ++digit) {
      value = static_cast<uint16_t>(value * 10 + (buffer_[offset + digit] - '0'));
    }
    if (value < kMinimumChannel || value > kMaximumChannel) {
      return false;
    }
    candidate.channels[channel] = value;
  }

  output = candidate;
  return true;
}

void V7RCProtocol::restartFrom(uint8_t byte) {
  if (byte == 'S') {
    buffer_[0] = 'S';
    index_ = 1;
  } else {
    index_ = 0;
  }
}
