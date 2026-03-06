# SmashOrPass
SmashOrPass - The Game

# Build

Windows:
- double-click bootstrap_build_windows.bat
- it will try to install Git, CMake, and Visual Studio Build Tools via winget
- then it clones/bootstraps vcpkg, configures CMake, and builds Debug

Linux (Ubuntu/Debian):
- run: chmod +x bootstrap_build_linux.sh
- then run: ./bootstrap_build_linux.sh
- it will install build tools with apt, clone/bootstraps vcpkg, configure, and build

Limits:
- Windows script assumes winget exists and that package installs are allowed.
- Linux script currently targets apt-based distros.
- Both scripts assume the project folder already contains the CMake project files.
