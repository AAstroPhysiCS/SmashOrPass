@echo off
setlocal EnableExtensions

call "%~dp0common.bat"
if errorlevel 1 exit /b %errorlevel%

set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=debug"

pushd "%PROJECT_ROOT%" >nul
if errorlevel 1 exit /b %errorlevel%

if /i "%CONFIG%"=="debug" (
    call :build_config debug
) else if /i "%CONFIG%"=="release" (
    call :build_config release
) else if /i "%CONFIG%"=="all" (
    call :build_config debug
    if not errorlevel 1 call :build_config release
) else (
    call :usage
    popd >nul
    exit /b 1
)

set "RESULT=%errorlevel%"
popd >nul
exit /b %RESULT%

:usage
echo Usage: %~nx0 [debug^|release^|all] 1>&2
exit /b 0

:build_config
cmake --build --preset %~1 --parallel
exit /b %errorlevel%
