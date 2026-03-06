#!/usr/bin/env bash
set -euo pipefail

PROJECT_NAME="SmashOrPass"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_PATH="$SCRIPT_DIR"
DEV_ROOT="${HOME}/dev"
VCPKG_ROOT="${VCPKG_ROOT:-${DEV_ROOT}/vcpkg}"
BUILD_DIR="${PROJECT_PATH}/build"
BUILD_TYPE="Debug"
GENERATOR="Ninja"
APT_UPDATED=0

say() {
  printf '\n==> %s\n' "$1"
}

info() {
  printf '[INFO] %s\n' "$1"
}

warn() {
  printf '[WARN] %s\n' "$1"
}

fail() {
  printf '[ERROR] %s\n' "$1" >&2
  exit 1
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

need_sudo() {
  if [ "$(id -u)" -eq 0 ]; then
    return 1
  fi
  if have_cmd sudo; then
    return 0
  fi
  return 1
}

run_root() {
  if need_sudo; then
    sudo "$@"
  else
    "$@"
  fi
}

apt_update_once() {
  if [ "$APT_UPDATED" -eq 0 ]; then
    say "Updating apt package index"
    run_root apt-get update
    APT_UPDATED=1
  fi
}

ensure_apt_pkg() {
  local cmd="$1"
  local pkg="$2"
  if have_cmd "$cmd"; then
    info "$cmd already installed."
    return 0
  fi
  have_cmd apt-get || fail "Automatic install requires apt-get on this Linux script."
  apt_update_once
  say "Installing ${pkg}"
  run_root apt-get install -y "$pkg"
  have_cmd "$cmd" || fail "${pkg} was installed but ${cmd} is still unavailable."
}

ensure_base_tools() {
  ensure_apt_pkg git git
  ensure_apt_pkg cmake cmake
  ensure_apt_pkg g++ g++
  ensure_apt_pkg ninja ninja-build
  ensure_apt_pkg pkg-config pkg-config
  ensure_apt_pkg make make
  ensure_apt_pkg curl curl
  ensure_apt_pkg tar tar
  ensure_apt_pkg unzip unzip
  ensure_apt_pkg zip zip
}

ensure_vcpkg_prereqs() {
  have_cmd apt-get || return 0
  local packages=(ca-certificates build-essential)
  apt_update_once
  say "Ensuring vcpkg build prerequisites"
  run_root apt-get install -y "${packages[@]}"
}

ensure_vcpkg() {
  mkdir -p "$DEV_ROOT"
  if [ -x "$VCPKG_ROOT/vcpkg" ]; then
    info "vcpkg already present at $VCPKG_ROOT"
  else
    if [ -d "$VCPKG_ROOT/.git" ]; then
      info "Existing vcpkg checkout found."
      say "Updating vcpkg"
      git -C "$VCPKG_ROOT" pull --ff-only || warn "Could not update vcpkg; continuing with existing checkout."
    elif [ -d "$VCPKG_ROOT" ]; then
      warn "Directory $VCPKG_ROOT exists but is not a git checkout. Reusing it."
    else
      say "Cloning vcpkg"
      git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
    fi
  fi

  if [ ! -x "$VCPKG_ROOT/vcpkg" ]; then
    say "Bootstrapping vcpkg"
    "$VCPKG_ROOT/bootstrap-vcpkg.sh"
  else
    info "vcpkg executable already available."
  fi

  export VCPKG_ROOT
  [ -x "$VCPKG_ROOT/vcpkg" ] || fail "vcpkg bootstrap did not produce an executable."
}

configure_project() {
  local toolchain="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
  [ -f "$toolchain" ] || fail "vcpkg toolchain file not found at $toolchain"
  say "Configuring ${PROJECT_NAME}"
  if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    info "Existing CMake cache found. Re-configuring in place."
  fi
  cmake -S "$PROJECT_PATH" -B "$BUILD_DIR" \
    -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$toolchain"
}

build_project() {
  say "Building ${PROJECT_NAME}"
  cmake --build "$BUILD_DIR"
}

main() {
  have_cmd apt-get || fail "This script currently supports Debian/Ubuntu style systems with apt-get."
  ensure_base_tools
  ensure_vcpkg_prereqs
  ensure_vcpkg
  configure_project
  build_project
  printf '\n[OK] Build directory: %s\n' "$BUILD_DIR"
}

main "$@"
