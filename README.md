# SmashOrPass
SmashOrPass - The Game

# Build

## Linux

Prerequisites:
- git
- cmake 3.25 or newer
- ninja
- a C++23 compiler
- clang-format for formatting

Bootstrap vcpkg and configure Debug/Release:

```sh
./scripts_linux/bootstrap.sh
```

Build:

```sh
./scripts_linux/build.sh          # Debug
./scripts_linux/build.sh release  # Release
./scripts_linux/build.sh all      # Debug and Release
```

Run:

```sh
./scripts_linux/run.sh          # Debug
./scripts_linux/run.sh release  # Release
```

Clean:

```sh
./scripts_linux/clean.sh
```

Format:

```sh
./scripts_linux/format.sh
```

## Windows

Prerequisites:
- git
- Visual Studio 2026 or Build Tools 2026 with Desktop development with C++
- clang-format for formatting, from the Visual Studio LLVM tools component

The Windows scripts are designed to run from a normal terminal. They find Visual Studio 2026 with
`vswhere`, initialize the x64 developer environment, and use the Visual Studio bundled CMake and
Ninja tools.

Bootstrap vcpkg and configure Debug/Release:

```bat
scripts_windows\bootstrap.bat
```

Build:

```bat
scripts_windows\build.bat          # Debug
scripts_windows\build.bat release  # Release
scripts_windows\build.bat all      # Debug and Release
```

Run:

```bat
scripts_windows\run.bat          # Debug
scripts_windows\run.bat release  # Release
```

Clean:

```bat
scripts_windows\clean.bat
```

Format:

```bat
scripts_windows\format.bat
```

Visual Studio 2026 can also open this repository as a CMake folder and use the existing
`CMakePresets.json` presets.
