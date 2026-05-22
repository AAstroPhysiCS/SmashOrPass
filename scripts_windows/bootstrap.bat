@echo off
setlocal EnableExtensions

call "%~dp0common.bat"
if errorlevel 1 exit /b %errorlevel%

set "VCPKG_ROOT=%PROJECT_ROOT%\.vcpkg"

pushd "%PROJECT_ROOT%" >nul
if errorlevel 1 exit /b %errorlevel%

if not exist "%VCPKG_ROOT%\" (
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "%VCPKG_ROOT%"
    if errorlevel 1 (
        popd >nul
        exit /b %errorlevel%
    )
) else if not exist "%VCPKG_ROOT%\.git\" (
    echo Expected %VCPKG_ROOT% to be a vcpkg git checkout. 1>&2
    popd >nul
    exit /b 1
) else (
    echo Using existing vcpkg checkout: %VCPKG_ROOT%
)

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat"
    if errorlevel 1 (
        popd >nul
        exit /b %errorlevel%
    )
)

cmake --preset debug
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

cmake --preset release
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

echo.
echo Bootstrap complete.
echo Build Debug:   %SCRIPT_DIR%build.bat debug
echo Build Release: %SCRIPT_DIR%build.bat release

popd >nul
exit /b 0
