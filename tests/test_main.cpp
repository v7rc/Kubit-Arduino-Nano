#include <assert.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>

#include "control_math.h"
#include "v7rc_protocol.h"

namespace {
std::vector<V7RCPacket> feedText(V7RCProtocol& parser,
                                 const std::string& text) {
  std::vector<V7RCPacket> packets;
  for (size_t i = 0; i < text.size(); ++i) {
    V7RCPacket packet = {{999, 999, 999, 999}};
    if (parser.feed(static_cast<uint8_t>(text[i]), packet)) {
      packets.push_back(packet);
    }
  }
  return packets;
}

void testValidAndContinuousPackets() {
  V7RCProtocol parser;
  const std::vector<V7RCPacket> packets = feedText(
      parser, "SRT1500150010002000#SRT2000100015001500#");
  assert(packets.size() == 2);
  assert(packets[0].channels[0] == 1500);
  assert(packets[0].channels[2] == 1000);
  assert(packets[0].channels[3] == 2000);
  assert(packets[1].channels[0] == 2000);
  assert(packets[1].channels[1] == 1000);
}

void testNoisePartialAndResynchronization() {
  V7RCProtocol parser;
  assert(feedText(parser, "noiseSRT15001500").empty());
  std::vector<V7RCPacket> packets = feedText(parser, "15001500#");
  assert(packets.size() == 1);

  packets = feedText(parser,
                     "SRSRT1500150015001500#"
                     "SRT150X150015001500#"
                     "SRT1000100010001000#");
  assert(packets.size() == 2);
  assert(packets[0].channels[0] == 1500);
  assert(packets[1].channels[0] == 1000);
}

void testEveryPacketSplitBoundary() {
  const std::string packetText = "SRT1234156711111999#";
  for (size_t split = 0; split < packetText.size(); ++split) {
    V7RCProtocol parser;
    assert(feedText(parser, packetText.substr(0, split)).empty());
    const std::vector<V7RCPacket> packets =
        feedText(parser, packetText.substr(split) + "\r\n");
    assert(packets.size() == 1);
    assert(packets[0].channels[0] == 1234);
    assert(packets[0].channels[1] == 1567);
    assert(packets[0].channels[2] == 1111);
    assert(packets[0].channels[3] == 1999);
  }
}

void testInvalidPacketsDoNotChangeOutput() {
  V7RCProtocol parser;
  V7RCPacket output = {{1111, 1222, 1333, 1444}};
  const std::string invalid[] = {
      "ABC1500150015001500#", "SRT0999150015001500#",
      "SRT2001150015001500#", "SRT1500150015001500!",
      "SRT15001500150A1500#"};

  for (size_t packetIndex = 0;
       packetIndex < sizeof(invalid) / sizeof(invalid[0]); ++packetIndex) {
    for (size_t i = 0; i < invalid[packetIndex].size(); ++i) {
      assert(!parser.feed(static_cast<uint8_t>(invalid[packetIndex][i]), output));
    }
    assert(output.channels[0] == 1111);
    assert(output.channels[3] == 1444);
  }
}

void testMotorMapping() {
  MotorCommand command = makeMotorCommand(1500, false, 25);
  assert(command.pwm == 0);
  assert(command.directionHigh);
  assert(makeMotorCommand(1475, false, 25).pwm == 0);
  assert(makeMotorCommand(1525, false, 25).pwm == 0);

  command = makeMotorCommand(2000, false, 25);
  assert(command.pwm == 255 && command.directionHigh);
  command = makeMotorCommand(1000, false, 25);
  assert(command.pwm == 255 && !command.directionHigh);
  command = makeMotorCommand(2000, true, 25);
  assert(command.pwm == 255 && !command.directionHigh);
  command = makeMotorCommand(1750, false, 25);
  assert(command.pwm > 0 && command.pwm < 255);
}
}  // namespace

int main() {
  testValidAndContinuousPackets();
  testNoisePartialAndResynchronization();
  testEveryPacketSplitBoundary();
  testInvalidPacketsDoNotChangeOutput();
  testMotorMapping();
  std::cout << "All V7RC protocol and control tests passed.\n";
  return 0;
}
