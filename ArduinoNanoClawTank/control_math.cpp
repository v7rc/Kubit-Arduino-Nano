#include "control_math.h"

namespace {
constexpr uint16_t kMinimumChannel = 1000;
constexpr uint16_t kCenterChannel = 1500;
constexpr uint16_t kMaximumChannel = 2000;

uint8_t scaleToPwm(uint16_t magnitude, uint16_t span) {
  if (magnitude >= span) {
    return 255;
  }
  return static_cast<uint8_t>((static_cast<uint32_t>(magnitude) * 255U) / span);
}
}  // namespace

MotorCommand makeMotorCommand(uint16_t channelValue, bool reversed,
                              uint16_t deadband) {
  if (channelValue < kMinimumChannel) {
    channelValue = kMinimumChannel;
  } else if (channelValue > kMaximumChannel) {
    channelValue = kMaximumChannel;
  }

  const uint16_t lowerStop = kCenterChannel - deadband;
  const uint16_t upperStop = kCenterChannel + deadband;
  bool forward = true;
  uint8_t pwm = 0;

  if (channelValue < lowerStop) {
    forward = false;
    pwm = scaleToPwm(lowerStop - channelValue, lowerStop - kMinimumChannel);
  } else if (channelValue > upperStop) {
    forward = true;
    pwm = scaleToPwm(channelValue - upperStop, kMaximumChannel - upperStop);
  }

  MotorCommand command = {pwm, static_cast<bool>(forward != reversed)};
  return command;
}
