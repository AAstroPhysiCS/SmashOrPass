@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"
set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=debug"

if /i "%CONFIG%"=="debug" (
    call "%SCRIPT_DIR%build.bat" debug
) else if /i "%CONFIG%"=="release" (
    call "%SCRIPT_DIR%build.bat" release
) else (
    call :usage
    exit /b 1
)

if errorlevel 1 exit /b %errorlevel%

pushd "%PROJECT_ROOT%" >nul
if errorlevel 1 exit /b %errorlevel%

"%PROJECT_ROOT%\build\%CONFIG%\smashorpass.exe"
set "RESULT=%errorlevel%"

popd >nul
exit /b %RESULT%

:usage
echo Usage: %~nx0 [debug^|release] 1>&2
exit /b 0
