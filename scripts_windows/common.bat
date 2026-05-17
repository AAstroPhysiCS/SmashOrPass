@echo off

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
)

if not exist "%VSWHERE%" (
    echo Missing required command: vswhere.exe 1>&2
    echo Install Visual Studio 2026 or Build Tools 2026 with Desktop development with C++. 1>&2
    exit /b 1
)

for %%I in ("%VSWHERE%") do set "VSWHERE_CMD=%%~sI"

set "VSINSTALL="
for /f "delims=" %%I in ('%VSWHERE_CMD% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -version "[18.0,18.999]" -property installationPath') do (
    set "VSINSTALL=%%I"
)

if not defined VSINSTALL (
    echo Missing required toolchain: Visual Studio 2026 or Build Tools 2026 with MSVC x64 tools. 1>&2
    exit /b 1
)

set "VSDEVCMD=%VSINSTALL%\Common7\Tools\VsDevCmd.bat"
if not exist "%VSDEVCMD%" (
    echo Expected Visual Studio developer command script was not found: %VSDEVCMD% 1>&2
    exit /b 1
)

call "%VSDEVCMD%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 (
    echo Failed to initialize the Visual Studio 2026 x64 developer environment. 1>&2
    exit /b 1
)

set "VS_CMAKE=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
set "VS_NINJA=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
set "VS_CLANG_FORMAT=%VSINSTALL%\VC\Tools\Llvm\x64\bin"

if exist "%VS_CMAKE%\cmake.exe" set "PATH=%VS_CMAKE%;%PATH%"
if exist "%VS_NINJA%\ninja.exe" set "PATH=%VS_NINJA%;%PATH%"
if /i "%~1"=="format" if exist "%VS_CLANG_FORMAT%\clang-format.exe" set "PATH=%VS_CLANG_FORMAT%;%PATH%"

where git >nul 2>nul
if errorlevel 1 (
    echo Missing required command: git 1>&2
    exit /b 1
)

where cmake >nul 2>nul
if errorlevel 1 (
    echo Missing required command: cmake 1>&2
    exit /b 1
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo Missing required command: ninja 1>&2
    exit /b 1
)

if /i "%~1"=="format" (
    where clang-format >nul 2>nul
    if errorlevel 1 (
        echo Missing required command: clang-format 1>&2
        exit /b 1
    )
)

exit /b 0
