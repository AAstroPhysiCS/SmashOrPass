#pragma once

#include <SDL3/SDL_rect.h>

#include <span>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Arena.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/SpriteAnimationPlayer.hpp"

namespace sop {

inline constexpr float kDefaultPlayerScreenWidth = kDefaultArenaWidth;
inline constexpr float kDefaultPlayerScreenHeight = kDefaultArenaHeight;
inline constexpr float kDefaultPlayerCollisionWidth = 116.0f;
inline constexpr float kDefaultPlayerCollisionHeight = 192.0f;
inline constexpr float kDefaultPlayerStartX = 450.0f;
inline constexpr float kDefaultPlayerGroundY = 448.0f;
inline constexpr float kDefaultPlayerFloorLineY =
    kDefaultPlayerGroundY + kDefaultPlayerCollisionHeight;
inline constexpr float kDefaultPlayerFloorLineRatio =
    kDefaultPlayerFloorLineY / kDefaultPlayerScreenHeight;
inline constexpr float kDefaultPlayerRenderHeight = kDefaultPlayerCollisionHeight;
inline constexpr float kDefaultPlayerReferenceSourceHeight = 482.0f;
inline constexpr float kDefaultPlayerRenderScale =
    kDefaultPlayerRenderHeight / kDefaultPlayerReferenceSourceHeight;

struct PlayerControlConfig {
    float GroundY = kDefaultPlayerGroundY;
    float MinX = 0.0f;
    float MaxX = kDefaultPlayerScreenWidth - kDefaultPlayerCollisionWidth;
    float MoveSpeed = 500.0f;
    float JumpVelocity = -900.0f;
    float Gravity = 2400.0f;
    double AttackSeconds = 0.25;
    float RenderScale = kDefaultPlayerRenderScale;
};

struct PlayerInputState {
    bool MoveLeft = false;
    bool MoveRight = false;
    bool JumpRequested = false;
    bool AttackHeld = false;
};

struct PlayerCharacterState {
    CharacterId Character = kDefaultCharacterId;
    SpriteAnimationPlayer Animation{CharacterAnimation::Idle};
    SDL_FPoint AnchorPosition{kDefaultPlayerStartX + (kDefaultPlayerCollisionWidth * 0.5f),
                              kDefaultPlayerGroundY + (kDefaultPlayerCollisionHeight * 0.5f)};
    SDL_FRect CollisionRect{kDefaultPlayerStartX,
                            kDefaultPlayerGroundY,
                            kDefaultPlayerCollisionWidth,
                            kDefaultPlayerCollisionHeight};
    SDL_FPoint CollisionAnchorOffset{kDefaultPlayerCollisionWidth * 0.5f,
                                     kDefaultPlayerCollisionHeight * 0.5f};
    SDL_FPoint FlippedCollisionAnchorOffset{kDefaultPlayerCollisionWidth * 0.5f,
                                            kDefaultPlayerCollisionHeight * 0.5f};
    float VerticalVelocity = 0.0f;
    bool Grounded = true;
    bool FacingRight = true;
    bool CollisionProfileInitialized = false;
    double AttackSecondsRemaining = 0.0;
};

void ApplyPlayerCollisionProfile(PlayerCharacterState& player,
                                 const SpriteSheetFrame& frame,
                                 const PlayerControlConfig& config = PlayerControlConfig{});

void ApplyPlayerKeyEvent(PlayerInputState& input,
                         PlayerCharacterState& player,
                         const KeyEvent& event,
                         const PlayerControlConfig& config = PlayerControlConfig{});

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config = PlayerControlConfig{});

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config,
                std::span<const SDL_FRect> floorPlatforms);

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         SDL_FPoint logicalViewportSize);

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         const SDL_FRect& arenaRect);

[[nodiscard]] CharacterAnimation SelectPlayerAnimation(const PlayerCharacterState& player,
                                                       const PlayerInputState& input);

}  // namespace sop
