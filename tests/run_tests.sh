#!/usr/bin/env sh
set -eu

PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TEST_BINARY="${TMPDIR:-/tmp}/arduino-nano-claw-tank-tests"

c++ -std=c++11 -Wall -Wextra -Werror \
  -I"$PROJECT_ROOT/ArduinoNanoClawTank" \
  "$PROJECT_ROOT/tests/test_main.cpp" \
  "$PROJECT_ROOT/ArduinoNanoClawTank/v7rc_protocol.cpp" \
  "$PROJECT_ROOT/ArduinoNanoClawTank/control_math.cpp" \
  -o "$TEST_BINARY"

"$TEST_BINARY"
