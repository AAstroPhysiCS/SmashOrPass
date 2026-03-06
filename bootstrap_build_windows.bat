@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Idempotent bootstrap/build script for SmashOrPass on Windows.
REM Place this file in the project root and double-click it, or run from cmd.

set "PROJECT_NAME=SmashOrPass"
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "PROJECT_PATH=%SCRIPT_DIR%"
set "DEV_ROOT=%USERPROFILE%\dev"
set "VCPKG_ROOT=%DEV_ROOT%\vcpkg"
set "BUILD_DIR=%PROJECT_PATH%\build"
set "CONFIG=Debug"
set "GENERATOR=Visual Studio 18 2026"
set "ARCH=x64"
set "VSDEVCMD="

call :banner
call :ensure_dir "%DEV_ROOT%" || goto :fail

call :ensure_git || goto :fail
call :refresh_path
call :ensure_cmake || goto :fail
call :refresh_path
call :ensure_vs_build_tools || goto :fail
call :ensure_vcpkg || goto :fail

set "TOOLCHAIN=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
if not exist "%TOOLCHAIN%" (
  echo [ERROR] vcpkg toolchain file not found: "%TOOLCHAIN%"
  goto :fail
)

call :find_vsdevcmd || goto :fail
call "%VSDEVCMD%" -arch=%ARCH% -host_arch=%ARCH%
if errorlevel 1 (
  echo [ERROR] Failed to initialize Visual Studio build environment.
  goto :fail
)

pushd "%PROJECT_PATH%" || goto :fail

call :configure_project || goto :fail_pop
call :build_project || goto :fail_pop

popd

echo.
echo [OK] %PROJECT_NAME% build completed.
echo [INFO] Build directory: "%BUILD_DIR%"
call :pause_if_double_clicked
exit /b 0

:configure_project
echo.
echo [STEP] Configuring %PROJECT_NAME%...
if exist "%BUILD_DIR%\CMakeCache.txt" (
  echo [INFO] Existing CMake cache found. Re-configuring in place.
)
cmake -S . -B "%BUILD_DIR%" -G "%GENERATOR%" -A %ARCH% -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%"
if errorlevel 1 (
  echo [ERROR] CMake configure failed.
  exit /b 1
)
exit /b 0

:build_project
echo.
echo [STEP] Building %PROJECT_NAME% (%CONFIG%)...
cmake --build "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)
exit /b 0

:ensure_git
where git >nul 2>nul
if not errorlevel 1 (
  echo [INFO] Git already installed.
  exit /b 0
)

echo [WARN] Git not found.
call :require_winget_for_install "Git"
if errorlevel 1 exit /b 1

echo [STEP] Installing Git...
winget install --id Git.Git -e --source winget --accept-package-agreements --accept-source-agreements
if errorlevel 1 (
  echo [ERROR] Git installation failed.
  exit /b 1
)
call :refresh_path
where git >nul 2>nul
if not errorlevel 1 (
  echo [INFO] Git installed successfully.
  exit /b 0
)

echo [ERROR] Git still not found after installation.
exit /b 1

:ensure_cmake
where cmake >nul 2>nul
if not errorlevel 1 (
  echo [INFO] CMake already installed.
  exit /b 0
)

echo [WARN] CMake not found.
call :require_winget_for_install "CMake"
if errorlevel 1 exit /b 1

echo [STEP] Installing CMake...
winget install --id Kitware.CMake -e --source winget --accept-package-agreements --accept-source-agreements
if errorlevel 1 (
  echo [ERROR] CMake installation failed.
  exit /b 1
)
call :refresh_path
where cmake >nul 2>nul
if not errorlevel 1 (
  echo [INFO] CMake installed successfully.
  exit /b 0
)

echo [ERROR] CMake still not found after installation.
exit /b 1

:ensure_vs_build_tools
call :find_vsdevcmd
if not errorlevel 1 (
  echo [INFO] Visual Studio C++ build tools already installed.
  exit /b 0
)

echo [WARN] Visual Studio 2026 C++ build tools not found.
call :require_winget_for_install "Visual Studio Build Tools"
if errorlevel 1 exit /b 1

echo [STEP] Installing Visual Studio 2026 Build Tools with C++ workload...
winget install --id Microsoft.VisualStudio.2026.BuildTools -e --source winget --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" --accept-package-agreements --accept-source-agreements
if errorlevel 1 (
  echo [ERROR] Visual Studio Build Tools installation failed.
  exit /b 1
)
call :find_vsdevcmd
if not errorlevel 1 (
  echo [INFO] Visual Studio build tools installed successfully.
  exit /b 0
)

echo [ERROR] Visual Studio build tools still not found after installation.
exit /b 1

:ensure_vcpkg
if exist "%VCPKG_ROOT%\vcpkg.exe" (
  echo [INFO] vcpkg already present at "%VCPKG_ROOT%".
) else (
  if exist "%VCPKG_ROOT%" (
    echo [INFO] Existing vcpkg directory found without executable. Reusing directory.
  ) else (
    echo [STEP] Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git "%VCPKG_ROOT%"
    if errorlevel 1 (
      echo [ERROR] Failed to clone vcpkg.
      exit /b 1
    )
  )
)

echo [STEP] Bootstrapping vcpkg if needed...
pushd "%VCPKG_ROOT%" || exit /b 1
if exist "vcpkg.exe" (
  echo [INFO] vcpkg executable already exists.
) else (
  call bootstrap-vcpkg.bat
  if errorlevel 1 (
    popd
    echo [ERROR] vcpkg bootstrap failed.
    exit /b 1
  )
)
popd
set "VCPKG_ROOT=%VCPKG_ROOT%"
setx VCPKG_ROOT "%VCPKG_ROOT%" >nul 2>nul
exit /b 0

:find_vsdevcmd
set "VSDEVCMD="
for %%F in (
  "%ProgramFiles%\Microsoft Visual Studio\2026\Community\Common7\Tools\VsDevCmd.bat"
  "%ProgramFiles%\Microsoft Visual Studio\2026\Professional\Common7\Tools\VsDevCmd.bat"
  "%ProgramFiles%\Microsoft Visual Studio\2026\Enterprise\Common7\Tools\VsDevCmd.bat"
  "%ProgramFiles(x86)%\Microsoft Visual Studio\2026\BuildTools\Common7\Tools\VsDevCmd.bat"
) do (
  if exist %%~F set "VSDEVCMD=%%~F"
)
if defined VSDEVCMD exit /b 0

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
  for /f "usebackq delims=" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find Common7\Tools\VsDevCmd.bat`) do (
    if exist "%%~I" set "VSDEVCMD=%%~I"
  )
)
if defined VSDEVCMD exit /b 0
exit /b 1

:refresh_path
set "PATH=%PATH%;%ProgramFiles%\Git\cmd;%ProgramFiles%\CMake\bin;%ProgramFiles(x86)%\Git\cmd"
exit /b 0

:require_winget_for_install
where winget >nul 2>nul
if not errorlevel 1 exit /b 0

echo [ERROR] %~1 is missing and winget is not available for automatic installation.
echo [ERROR] Install %~1 manually, then run this script again.
exit /b 1

:ensure_dir
if exist "%~1" exit /b 0
mkdir "%~1"
if errorlevel 1 (
  echo [ERROR] Could not create directory "%~1".
  exit /b 1
)
exit /b 0

:banner
echo ==========================================
echo   SmashOrPass Windows bootstrap and build
echo ==========================================
exit /b 0

:pause_if_double_clicked
if /i "%cmdcmdline:/c=%"=="%cmdcmdline%" pause
exit /b 0

:fail_pop
popd
:fail
echo.
echo [FAIL] Build process did not complete.
call :pause_if_double_clicked
exit /b 1
