#include "control_math.h"

namespace {
constexpr uint16_t kMinimumChannel = 1000;
constexpr uint16_t kCenterChannel = 1500;
constexpr uint16_t kMaximumChannel = 2000;
constexpr int16_t kMaximumDemand = 500;

uint16_t constrainChannel(uint16_t value) {
  if (value < kMinimumChannel) {
    return kMinimumChannel;
  }
  if (value > kMaximumChannel) {
    return kMaximumChannel;
  }
  return value;
}

int16_t constrainDemand(int32_t value) {
  if (value < -kMaximumDemand) {
    return -kMaximumDemand;
  }
  if (value > kMaximumDemand) {
    return kMaximumDemand;
  }
  return static_cast<int16_t>(value);
}

int16_t channelToAxis(uint16_t channelValue, uint16_t deadband) {
  channelValue = constrainChannel(channelValue);
  if (deadband >= kMaximumDemand) {
    deadband = kMaximumDemand - 1;
  }

  const uint16_t lowerStop = kCenterChannel - deadband;
  const uint16_t upperStop = kCenterChannel + deadband;
  if (channelValue >= lowerStop && channelValue <= upperStop) {
    return 0;
  }

  if (channelValue > upperStop) {
    const uint16_t magnitude = channelValue - upperStop;
    const uint16_t span = kMaximumChannel - upperStop;
    return static_cast<int16_t>(
        (static_cast<uint32_t>(magnitude) * kMaximumDemand + span / 2) / span);
  }

  const uint16_t magnitude = lowerStop - channelValue;
  const uint16_t span = lowerStop - kMinimumChannel;
  return -static_cast<int16_t>(
      (static_cast<uint32_t>(magnitude) * kMaximumDemand + span / 2) / span);
}
}  // namespace

TankDriveMix mixTankDrive(uint16_t steeringChannel, uint16_t throttleChannel,
                          uint16_t deadband, bool steeringReversed) {
  int16_t steering = channelToAxis(steeringChannel, deadband);
  const int16_t throttle = channelToAxis(throttleChannel, deadband);
  if (steeringReversed) {
    steering = -steering;
  }

  // CH2 is forward/reverse throttle; CH1 adds opposite corrections to the two
  // sides. Saturation keeps every motor demand in the supported range.
  TankDriveMix mix = {
      constrainDemand(static_cast<int32_t>(throttle) + steering),
      constrainDemand(static_cast<int32_t>(throttle) - steering)};
  return mix;
}

MotorCommand makeMotorCommand(int16_t motorDemand, bool reversed) {
  motorDemand = constrainDemand(motorDemand);
  if (reversed) {
    motorDemand = -motorDemand;
  }

  if (motorDemand == 0) {
    MotorCommand stopped = {0, false, true};
    return stopped;
  }

  const bool forward = motorDemand > 0;
  const uint16_t magnitude = static_cast<uint16_t>(
      forward ? motorDemand : -static_cast<int32_t>(motorDemand));
  const uint8_t drivePwm = static_cast<uint8_t>(
      (static_cast<uint32_t>(magnitude) * 255U + kMaximumDemand / 2) /
      kMaximumDemand);

  // Carrier truth table:
  //   DIR=0, PWM=0 -> stop       DIR=1, PWM=0 -> full forward
  //   DIR=0, PWM=1 -> reverse    DIR=1, PWM=1 -> brake
  // Forward uses inverted PWM (forward/brake); reverse uses normal PWM
  // (reverse/coast).
  MotorCommand command = {
      static_cast<uint8_t>(forward ? 255U - drivePwm : drivePwm), forward,
      false};
  return command;
}
