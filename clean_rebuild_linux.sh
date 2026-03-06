#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

echo "[INFO] Project root: $(pwd)"

if [[ ! -f "CMakeLists.txt" ]]; then
    echo "[ERROR] CMakeLists.txt not found. Put this script in the project root."
    exit 1
fi

echo
echo "[INFO] Cleaning generated files..."

if [[ -d "build" ]]; then
    rm -rf build
    echo "[OK] Removed build/"
else
    echo "[SKIP] build/ not found"
fi

if [[ -d "vcpkg_installed" ]]; then
    rm -rf vcpkg_installed
    echo "[OK] Removed vcpkg_installed/"
else
    echo "[SKIP] vcpkg_installed/ not found"
fi

if [[ -d ".cache" ]]; then
    rm -rf .cache
    echo "[OK] Removed .cache/"
else
    echo "[SKIP] .cache/ not found"
fi

echo
echo "[INFO] Reconfiguring and rebuilding..."

if [[ ! -d "vcpkg" ]]; then
    echo "[ERROR] Local vcpkg folder not found at $(pwd)/vcpkg"
    echo "[ERROR] Run your bootstrap script first, or clone vcpkg into the project."
    exit 1
fi

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j

echo
echo "[SUCCESS] Clean rebuild completed."