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
PKG_MANAGER=""
PKG_DB_SYNCED=0

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

detect_pkg_manager() {
  local os_id="" os_like=""

  if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    os_id="${ID:-}"
    os_like="${ID_LIKE:-}"
  fi

  case " ${os_id} ${os_like} " in
    *" arch "*)
      PKG_MANAGER="pacman"
      ;;
    *" debian "*|*" ubuntu "*)
      PKG_MANAGER="apt"
      ;;
    *)
      if have_cmd apt-get; then
        PKG_MANAGER="apt"
      elif have_cmd pacman; then
        PKG_MANAGER="pacman"
      else
        fail "Unsupported system. This script currently supports Debian/Ubuntu (apt) and Arch Linux (pacman)."
      fi
      ;;
  esac

  info "Using package manager: ${PKG_MANAGER}"
}

pkg_update_once() {
  if [ "$PKG_DB_SYNCED" -ne 0 ]; then
    return 0
  fi

  case "$PKG_MANAGER" in
    apt)
      say "Updating apt package index"
      run_root apt-get update
      ;;
    pacman)
      say "Synchronizing pacman databases and upgrading installed packages"
      run_root pacman -Syu --noconfirm
      ;;
    *)
      fail "Unknown package manager: $PKG_MANAGER"
      ;;
  esac

  PKG_DB_SYNCED=1
}

ensure_apt_pkg() {
  local cmd="$1"
  local pkg="$2"

  if have_cmd "$cmd"; then
    info "$cmd already installed."
    return 0
  fi

  pkg_update_once
  say "Installing ${pkg}"
  run_root apt-get install -y "$pkg"
  have_cmd "$cmd" || fail "${pkg} was installed but ${cmd} is still unavailable."
}

ensure_pacman_pkg() {
  local cmd="$1"
  local pkg="$2"

  if have_cmd "$cmd"; then
    info "$cmd already installed."
    return 0
  fi

  pkg_update_once
  say "Installing ${pkg}"
  run_root pacman -S --noconfirm --needed "$pkg"
  have_cmd "$cmd" || fail "${pkg} was installed but ${cmd} is still unavailable."
}

ensure_system_pkg() {
  local cmd="$1"
  local apt_pkg="$2"
  local pacman_pkg="$3"

  case "$PKG_MANAGER" in
    apt)
      ensure_apt_pkg "$cmd" "$apt_pkg"
      ;;
    pacman)
      ensure_pacman_pkg "$cmd" "$pacman_pkg"
      ;;
    *)
      fail "Unknown package manager: $PKG_MANAGER"
      ;;
  esac
}

ensure_base_tools() {
  ensure_system_pkg git git git
  ensure_system_pkg cmake cmake cmake
  ensure_system_pkg g++ g++ gcc
  ensure_system_pkg ninja ninja-build ninja
  ensure_system_pkg pkg-config pkg-config pkgconf
  ensure_system_pkg make make make
  ensure_system_pkg curl curl curl
  ensure_system_pkg tar tar tar
  ensure_system_pkg unzip unzip unzip
  ensure_system_pkg zip zip zip
}

ensure_vcpkg_prereqs() {
  case "$PKG_MANAGER" in
    apt)
      local packages=(
        ca-certificates
        build-essential
        autoconf
        autoconf-archive
        automake
        libtool
      )
      pkg_update_once
      say "Ensuring vcpkg build prerequisites"
      run_root apt-get install -y "${packages[@]}"
      ;;
    pacman)
      local packages=(
        ca-certificates
        base-devel
        autoconf
        autoconf-archive
        automake
        libtool
      )
      pkg_update_once
      say "Ensuring vcpkg build prerequisites"
      run_root pacman -S --noconfirm --needed "${packages[@]}"
      ;;
    *)
      fail "Unknown package manager: $PKG_MANAGER"
      ;;
  esac
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
      git clone --depth 1 https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
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
  detect_pkg_manager
  ensure_base_tools
  ensure_vcpkg_prereqs
  ensure_vcpkg
  configure_project
  build_project
  printf '\n[OK] Build directory: %s\n' "$BUILD_DIR"
}

main "$@"
