#!/usr/bin/env bash
set -euo pipefail

cmake --preset native-debug
cmake --build --preset native-debug -j
exec ./build/native-debug/qt_hello
