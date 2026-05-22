#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CONFIG="${1:-debug}"

usage() {
    printf 'Usage: %s [debug|release]\n' "$0" >&2
}

case "${CONFIG}" in
    debug|release)
        "${SCRIPT_DIR}/build.sh" "${CONFIG}"
        cd "${PROJECT_ROOT}"
        exec "${PROJECT_ROOT}/build/${CONFIG}/smashorpass"
        ;;
    *)
        usage
        exit 1
        ;;
esac
