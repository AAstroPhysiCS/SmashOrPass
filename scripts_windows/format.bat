@echo off
setlocal EnableExtensions

call "%~dp0common.bat" format
if errorlevel 1 exit /b %errorlevel%

pushd "%PROJECT_ROOT%" >nul
if errorlevel 1 exit /b %errorlevel%

for /r "include" %%F in (*.hpp *.h *.cpp *.cc *.cxx) do clang-format -i "%%F"
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

for /r "src" %%F in (*.hpp *.h *.cpp *.cc *.cxx) do clang-format -i "%%F"
if errorlevel 1 (
    popd >nul
    exit /b %errorlevel%
)

popd >nul
exit /b 0
