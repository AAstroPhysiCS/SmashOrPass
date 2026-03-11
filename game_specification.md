# Game Specification

We describe a simple 2D platform fighter inspired by games such as *Super Smash Bros.*.
It is a local multiplayer game in which the objective is to defeat the opponent either by throwing them off the stage or by reducing their HP to 0, depending on the selected game mode. The first player to achieve this **n** times wins the match.

Players can choose from different characters, each with unique stats, moves, and abilities.

For references, see:

* Super Smash Bros.
* Killer Instinct
* Street Fighter
* Mortal Kombat

## Assets

The game is based on 2D sprite rendering and uses sprite sheets for characters, effects, UI elements, and map objects.

Sprite sheets are created using external tools such as Aseprite, Blender, or Python scripts. Each sprite sheet is accompanied by a metadata JSON file that defines animation frames, frame durations, sprite regions, and other relevant rendering data.

Collision boxes, hurt boxes, and hit boxes are specified separately from the visual sprite sheet. One possible approach is to define them through a companion sprite sheet or metadata representation, for example:

* Red channel: hurt box
* Green channel: hit box
* Blue channel: collision box

The collision box should remain simple and rectangular for stable movement and collision handling.

The game also includes sound effects and background music.

Additionally, the game features a particle effect system for attacks, impacts, movement, and other visual feedback.

## World and Maps

Maps consist of background and foreground sprites and define the arena in which players fight.

Each map may contain static and moving elements such as platforms, hazards, or animated background details. Dynamic map elements are defined in companion JSON files so that map logic and data remain separate from the engine code.

Maps should differ not only visually but also in layout and gameplay feel.

## Rendering and Animation

All visible objects are rendered as sprites.

Each entity has a transform containing at least its position and, if needed, orientation and scale. This transform determines where and how the entity is rendered.

Animations are implemented by playing sequences of frames from a sprite sheet. These include idle, movement, jump, attack, hit, and defeat animations.

The metadata for each animation should define at least:

* Frame order
* Frame timing
* Looping behavior
* Optional events such as attack activation windows

## Combat and Game Logic

The game is built around fast, responsive real-time combat.

Characters can move, jump, fall, and attack. Attacks interact with hit boxes and hurt boxes to determine whether they connect. Depending on the selected game mode, attacks either contribute to knockback-based elimination, HP-based elimination, or both.

Characters have individual stats that may influence movement speed, jump height, attack power, knockback, defense, or other combat-related properties.

The game should support multiple game modes, for example:

* **Smash / Sumo**: win by knocking the opponent off the stage
* **Deathmatch**: win by reducing the opponent's HP to 0

The rules for victory are determined by the selected mode.

## Menus and Flow

A typical game session looks like this:

1. The player runs the game binary
2. The game enters a loading state and loads or parses required assets
3. The game enters the main menu
4. The players choose a game mode
5. The players select their characters
6. The players select or vote on a map
7. The match begins
8. The game enters a post-game results or statistics screen

The game also contains a settings screen. Settings should be saved to disk and restored on the next launch.

Runtime assets should be loaded robustly, and where appropriate, lazily.

## Audio

The game includes both music and sound effects.

Background music should be used for menus and matches. Sound effects should provide feedback for actions such as attacks, hits, jumps, UI interactions, stage events, and victory screens.

Audio playback should be managed independently from rendering and gameplay logic.

## AI

If feasible, the game may include enemy AI so that players can fight against computer-controlled opponents.

AI behavior does not need to be highly advanced, but it should be capable of navigating the stage, choosing attacks, and reacting to nearby opponents in a believable way.

## Debugging

Possible features include:

* Profiling and statistics menu
* Visualization of hit boxes, hurt boxes, and collision boxes
* Asset registry visualizer
* Cheats or testing helpers
* Fallback assets for missing sprites, sounds, or metadata

Fallback assets are especially useful during development. Instead of crashing when an asset is missing, the game should log a warning and use a placeholder asset instead.

---

## Goals (11 Points)

* **(2) Core match gameplay**

  * Local multiplayer match for 2 players on the same machine
  * Players can win either by knocking the opponent off the stage or by depleting their HP, depending on the selected game mode
  * A match consists of multiple rounds / stocks, and the first player to win **n** rounds wins the game

* **(1) Characters**

  * Players can choose from multiple playable characters
  * Each character has distinct stats and at least a small set of unique attacks / abilities

* **(1) Movement and combat mechanics**

  * Characters can move, jump, fall, and attack responsively
  * Attacks interact with knockback / damage / HP in a consistent way
  * A player is defeated when the selected game mode’s loss condition is met

* **(1) Collision and hit detection**

  * Functional collision handling with stage geometry
  * Hit boxes, hurt boxes, and collision boxes are used for combat and movement
  * Attacks only connect when the relevant boxes overlap correctly

* **(1) Maps**

  * At least 2 different maps
  * Maps differ in appearance and layout
  * At least one map contains an interactive or moving element such as a moving platform
  * Map-specific data is loaded from external metadata files

* **(1) Menus and game flow**

  * Loading screen
  * Main menu
  * Game mode selection
  * Character selection
  * Map selection
  * Post-game results / statistics screen

* **(1) Settings and persistence**

  * Dedicated settings screen
  * Settings are saved to disk and persist across play sessions

* **(1) Audio and visual feedback**

  * Background music
  * Sound effects for combat and UI actions
  * Particle effects for attacks, impacts, or stage events

* **(1) Asset system**

  * Sprites are rendered from sprite sheets
  * Animation and frame metadata are loaded from companion JSON files
  * Assets are loaded robustly, ideally with lazy loading where appropriate
  * Missing assets do not crash the game; fallback assets are used instead

* **(1) Debugging utilities**

  * Debug / profiling menu or overlay
  * Visualization of hit boxes, hurt boxes, and collision boxes
  * Basic runtime statistics or asset inspection tools
  * Optional cheat / testing helpers for development
