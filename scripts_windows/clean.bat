@echo off
setlocal EnableExtensions

call "%~dp0common.bat"
if errorlevel 1 exit /b %errorlevel%

pushd "%PROJECT_ROOT%" >nul
if errorlevel 1 exit /b %errorlevel%

cmake --build --preset debug --target clean
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

cmake --build --preset release --target clean
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

popd >nul
exit /b 0
