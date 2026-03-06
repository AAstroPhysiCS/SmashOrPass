#!/usr/bin/env bash
set -euo pipefail

find include src tests -type f \( -name '*.hpp' -o -name '*.cpp' \) -print0 | xargs -0 clang-format -i
