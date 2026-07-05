# SmashOrPass
SmashOrPass - The Game

# Responsibilities

| Person | Responsibilities | Related folders/files |
|---|---|---|
| [Okan Güclü](https://github.com/AAstroPhysiCS) | All UI-related systems, Audio-System, Renderer, Particle-System, Framemasks, and overall core. | [src/ui](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/ui), [include/smashorpass/ui](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/ui), [src/audio](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/audio), [include/smashorpass/audio](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/audio), [src/rendering](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/rendering), [include/smashorpass/rendering](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/rendering), [src/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/core), [include/smashorpass/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/core), [assets/particles](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/assets/particles), [asset/effects](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/asset/effects) |
| [Alexander Heim](https://github.com/alexdesander) | Asynchronous AssetManager, State stack system, sprite creation in Blender, AI-System, and overall core. | [src/asset](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/asset), [include/smashorpass/asset](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/asset), [src/state/StateManager.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/StateManager.cpp), [include/smashorpass/state](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/state), [assets/sprites](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/assets/sprites), [tools/assets/SpriteSheetCreation.txt](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/tools/assets/SpriteSheetCreation.txt), [src/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/core), [include/smashorpass/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/core), [include/smashorpass/state/states/in_game](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/state/states/in_game) |
| [Gian Alber](https://github.com/gianf1o) | Gameplay system, in-game logic, movement, collision, hitboxes/hurtboxes, rounds/stocks, match results, statistics and persistent settings. | [src/state/states/in_game](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/state/states/in_game), [include/smashorpass/state/states/in_game](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/state/states/in_game), [src/state/states/main_menu](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/state/states/main_menu), [src/state/states/match_results](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/state/states/match_results) |

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


