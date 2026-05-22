#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CONFIG="${1:-debug}"

cd "${PROJECT_ROOT}"

usage() {
    printf 'Usage: %s [debug|release|all]\n' "$0" >&2
}

build_config() {
    cmake --build --preset "$1" --parallel
}

case "${CONFIG}" in
    debug)
        build_config debug
        ;;
    release)
        build_config release
        ;;
    all)
        build_config debug
        build_config release
        ;;
    *)
        usage
        exit 1
        ;;
esac
