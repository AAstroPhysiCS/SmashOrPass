#pragma once

#include <SDL3/SDL_rect.h>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Arena.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/SpriteAnimationPlayer.hpp"

namespace sop {

inline constexpr float kDefaultPlayerScreenWidth = kDefaultArenaWidth;
inline constexpr float kDefaultPlayerScreenHeight = kDefaultArenaHeight;
inline constexpr float kDefaultPlayerPlaceholderWidth = 116.0f;
inline constexpr float kDefaultPlayerPlaceholderHeight = 192.0f;
inline constexpr float kDefaultPlayerStartX = 260.0f;
inline constexpr float kDefaultPlayerGroundY = 420.0f;
inline constexpr float kDefaultPlayerFloorLineY =
    kDefaultPlayerGroundY + kDefaultPlayerPlaceholderHeight;
inline constexpr float kDefaultPlayerFloorLineRatio =
    kDefaultPlayerFloorLineY / kDefaultPlayerScreenHeight;
inline constexpr float kDefaultRobotReferenceSourceHeight = 482.0f;

struct PlayerControlConfig {
    float GroundY = kDefaultPlayerGroundY;
    float MinX = 0.0f;
    float MaxX = kDefaultPlayerScreenWidth - kDefaultPlayerPlaceholderWidth;
    float MoveSpeed = 360.0f;
    float JumpVelocity = -900.0f;
    float Gravity = 2400.0f;
    double AttackSeconds = 0.25;
    float RenderReferenceSourceHeight = kDefaultRobotReferenceSourceHeight;
};

struct PlayerInputState {
    bool MoveLeft = false;
    bool MoveRight = false;
    bool JumpRequested = false;
};

struct PlayerCharacterState {
    CharacterId Character = CharacterId::Robot;
    SpriteAnimationPlayer Animation{CharacterAnimation::Idle};
    SDL_FRect PlaceholderRect{kDefaultPlayerStartX,
                              kDefaultPlayerGroundY,
                              kDefaultPlayerPlaceholderWidth,
                              kDefaultPlayerPlaceholderHeight};
    float VerticalVelocity = 0.0f;
    bool Grounded = true;
    bool FacingRight = true;
    double AttackSecondsRemaining = 0.0;
};

void ApplyPlayerKeyEvent(PlayerInputState& input,
                         PlayerCharacterState& player,
                         const KeyEvent& event,
                         const PlayerControlConfig& config = PlayerControlConfig{});

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config = PlayerControlConfig{});

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         SDL_FPoint logicalViewportSize);

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         const SDL_FRect& arenaRect);

[[nodiscard]] CharacterAnimation SelectPlayerAnimation(const PlayerCharacterState& player,
                                                       const PlayerInputState& input);

}  // namespace sop
