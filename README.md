# SmashOrPass
SmashOrPass - The Game

# Responsibilities

| Person                                            | Topic                              | Related folders/files                                                                                                                                                                                                                                    |
| ------------------------------------------------- | ---------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Okan Güclü](https://github.com/AAstroPhysiCS)    | All UI-related systems | [src/ui](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/ui), [include/smashorpass/ui](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/ui), [GameScreen and PauseScreen](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/ui), [MenuScreen](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/main_menu/ui/MenuScreen.cpp), [CharacterSelectScreen](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/main_menu/ui/CharacterSelectScreen.cpp), etc...
|                                                   | Audio-System / Asset               | [src/audio](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/audio), [include/smashorpass/audio](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/audio), [src/asset/assets/AudioAsset.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/asset/assets/AudioAsset.cpp)                                                            |
|                                                   | Renderer                           | [src/rendering](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/rendering), [include/smashorpass/rendering](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/rendering)                                             |
|                                                   | Particle-System                    | [assets/particles](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/assets/particles)                                                                                                                                                              |
|                                                   | Framemasks                         | [asset/effects](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/asset/effects)                                                                                                                                                |
|                                                   | Application, Window, Base          | [src/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/core), [include/smashorpass/core](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/core)                                                                 |
|                                                   | Event-System, Event-Dispatcher     | [include/smashorpass/core/Event.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/core/Event.hpp)                                            
| [Alexander Heim](https://github.com/alexdesander) | Asset manager system               | [include/smashorpass/asset/AssetManager.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/asset/AssetManager.hpp), [src/asset/AssetManager.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/asset/AssetManager.cpp), [include/smashorpass/core/ConcurrentQueue.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/core/ConcurrentQueue.hpp) |
|                                                   | AI Agent System                    | [src/state/states/in_game/AiAgent.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/AiAgent.cpp), [include/smashorpass/state/states/in_game/AiAgent.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/AiAgent.hpp)                 |
|                                                   | State Stack                        | [src/state/StateManager.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/StateManager.cpp), [include/smashorpass/state/StateManager.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/StateManager.hpp), [include/smashorpass/state/State.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/State.hpp) |
|                                                   | Input translation                  | [include/smashorpass/core/InputHelper.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/core/InputHelper.hpp), [src/core/InputHelper.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/core/InputHelper.cpp)  |
|                                                   | Error handling                     | [include/smashorpass/util.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/util.hpp), [src/core/Application.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/core/Application.cpp) |
| [Gian Alber](https://github.com/gianf1o)          | Gameplay system                    | [src/state/states/in_game](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/state/states/in_game), [include/smashorpass/state/states/in_game](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/state/states/in_game) |
|                                                   | in-game logic                      | [InGameState.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/InGameState.cpp), [InGameState.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/InGameState.hpp), [Player.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/Player.cpp) |
|                                                   | movement                           | [PlayerMovement.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/PlayerMovement.cpp), [PlayerMovement.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/PlayerMovement.hpp), [PlayerMovementIntegration.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/PlayerMovementIntegration.cpp) |
|                                                   | collision                          | [CollisionSystem.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/CollisionSystem.cpp), [CollisionSystem.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/CollisionSystem.hpp), [PlayerCollisionIntegration.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/PlayerCollisionIntegration.cpp) |
|                                                   | combat, hitboxes/hurtboxes         | [CombatSystem.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/CombatSystem.cpp), [CombatSystem.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/CombatSystem.hpp), [CharacterCombatData.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/asset/assets/CharacterCombatData.cpp), [CharacterCombatData.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/asset/assets/CharacterCombatData.hpp), [character combat masks](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/assets/sprites/characters) |
|                                                   | hit application and match stats    | [InGameCombatIntegration.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/InGameCombatIntegration.cpp), [PlayerCombatIntegration.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/PlayerCombatIntegration.cpp), [MatchStats.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/MatchStats.hpp) |
|                                                   | rounds/stocks and game rules       | [InGameRules.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/in_game/InGameRules.cpp), [GameMode.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/GameMode.hpp), [MatchConfig.hpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/include/smashorpass/state/states/in_game/MatchConfig.hpp) |
|                                                   | match results                      | [src/state/states/match_results](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/state/states/match_results), [include/smashorpass/state/states/match_results](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/state/states/match_results) |
|                                                   | statistics and persistent settings | [src/persistence](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/src/persistence), [include/smashorpass/persistence](https://github.com/AAstroPhysiCS/SmashOrPass/tree/main/include/smashorpass/persistence), [SettingsScreen.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/main_menu/ui/SettingsScreen.cpp), [KeybindSettingsScreen.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/main_menu/ui/KeybindSettingsScreen.cpp), [ScoreboardScreen.cpp](https://github.com/AAstroPhysiCS/SmashOrPass/blob/main/src/state/states/main_menu/ui/ScoreboardScreen.cpp) |

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


