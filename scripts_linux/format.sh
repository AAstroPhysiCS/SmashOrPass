#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

if ! command -v clang-format >/dev/null 2>&1; then
    printf 'Missing required command: clang-format\n' >&2
    exit 1
fi

find "${PROJECT_ROOT}/include" "${PROJECT_ROOT}/src" \
    -type f \( -name '*.hpp' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \) \
    -print0 | xargs -0 clang-format -i
