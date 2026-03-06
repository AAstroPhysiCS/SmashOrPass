@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

echo [INFO] Project root: %CD%

if not exist "CMakeLists.txt" (
    echo [ERROR] CMakeLists.txt not found. Put this script in the project root.
    exit /b 1
)

echo.
echo [INFO] Cleaning generated files...

if exist "build" (
    rmdir /s /q "build"
    echo [OK] Removed build\
) else (
    echo [SKIP] build\ not found
)

if exist "vcpkg_installed" (
    rmdir /s /q "vcpkg_installed"
    echo [OK] Removed vcpkg_installed\
) else (
    echo [SKIP] vcpkg_installed\ not found
)

if exist ".vs" (
    rmdir /s /q ".vs"
    echo [OK] Removed .vs\
) else (
    echo [SKIP] .vs\ not found
)

echo.
echo [INFO] Reconfiguring and rebuilding...

if not exist "vcpkg" (
    echo [ERROR] Local vcpkg folder not found at %CD%\vcpkg
    echo [ERROR] Run your bootstrap script first, or clone vcpkg into the project.
    exit /b 1
)

cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE="%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake"
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    exit /b 1
)

cmake --build build --config Debug
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo [SUCCESS] Clean rebuild completed.
exit /b 0