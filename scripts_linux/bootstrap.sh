#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
VCPKG_ROOT="${PROJECT_ROOT}/.vcpkg"

cd "${PROJECT_ROOT}"

have_cmd() {
    command -v "$1" >/dev/null 2>&1
}

need_cmd() {
    if ! have_cmd "$1"; then
        printf 'Missing required command: %s\n' "$1" >&2
        exit 1
    fi
}

need_cmd git
need_cmd cmake
need_cmd ninja

if [ ! -d "${VCPKG_ROOT}" ]; then
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
elif [ ! -d "${VCPKG_ROOT}/.git" ]; then
    printf 'Expected %s to be a vcpkg git checkout.\n' "${VCPKG_ROOT}" >&2
    exit 1
else
    printf 'Using existing vcpkg checkout: %s\n' "${VCPKG_ROOT}"
fi

if [ ! -x "${VCPKG_ROOT}/vcpkg" ]; then
    "${VCPKG_ROOT}/bootstrap-vcpkg.sh"
fi

cmake --preset debug
cmake --preset release

printf '\nBootstrap complete.\n'
printf 'Build Debug:   %s/build.sh debug\n' "${SCRIPT_DIR}"
printf 'Build Release: %s/build.sh release\n' "${SCRIPT_DIR}"
