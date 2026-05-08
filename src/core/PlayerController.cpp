#include "smashorpass/core/PlayerController.hpp"

#include <SDL3/SDL_keycode.h>

#include <algorithm>
#include <limits>
#include <optional>

#include "smashorpass/core/Base.hpp"
#include "smashorpass/core/InputBindings.hpp"
#include "smashorpass/rendering/ParticleSystem.hpp"

namespace sop {
namespace {

[[nodiscard]] SDL_FPoint CollisionAnchorOffsetFor(const PlayerCharacterState& player) {
    return player.FacingRight ? player.FlippedCollisionAnchorOffset : player.CollisionAnchorOffset;
}

void SyncCollisionRectToAnchor(PlayerCharacterState& player) {
    const SDL_FPoint offset = CollisionAnchorOffsetFor(player);
    player.CollisionRect.x = player.AnchorPosition.x - offset.x;
    player.CollisionRect.y = player.AnchorPosition.y - offset.y;
}

void TranslatePlayer(PlayerCharacterState& player, float dx, float dy) {
    player.AnchorPosition.x += dx;
    player.AnchorPosition.y += dy;
    player.CollisionRect.x += dx;
    player.CollisionRect.y += dy;
}

void SetPlayerCollisionX(PlayerCharacterState& player, float x) {
    TranslatePlayer(player, x - player.CollisionRect.x, 0.0f);
}

void SetPlayerCollisionY(PlayerCharacterState& player, float y) {
    TranslatePlayer(player, 0.0f, y - player.CollisionRect.y);
}

void TickTimer(double& secondsRemaining, double elapsedSeconds) {
    secondsRemaining = std::max(0.0, secondsRemaining - elapsedSeconds);
}

void ResetAirActions(PlayerCharacterState& player) {
    player.AirDashAvailable = true;
    player.AirJumpAvailable = false;
}

void LandPlayer(PlayerCharacterState& player, float y) {
    SetPlayerCollisionY(player, y);
    player.VerticalVelocity = 0.0f;
    player.Grounded = true;
    ResetAirActions(player);
}

[[nodiscard]] bool IsDashActive(const PlayerCharacterState& player) {
    return player.DashSecondsRemaining > 0.0;
}

[[nodiscard]] float SelectDashDirection(const PlayerCharacterState& player,
                                        const PlayerInputState& input) {
    if (input.MoveLeft != input.MoveRight) {
        return input.MoveLeft ? -1.0f : 1.0f;
    }

    return player.FacingRight ? 1.0f : -1.0f;
}

[[nodiscard]] bool HorizontallyOverlaps(const SDL_FRect& a, const SDL_FRect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x;
}

[[nodiscard]] std::optional<float> FindSupportedPlatformY(
    const SDL_FRect& playerRect, std::span<const SDL_FRect> floorPlatforms) {
    constexpr float kSupportEpsilon = 0.5f;

    const float playerBottom = playerRect.y + playerRect.h;
    float bestPlatformY = std::numeric_limits<float>::max();

    for (const SDL_FRect& platform : floorPlatforms) {
        if (!HorizontallyOverlaps(playerRect, platform)) {
            continue;
        }

        if (playerBottom < platform.y - kSupportEpsilon ||
            playerBottom > platform.y + kSupportEpsilon) {
            continue;
        }

        bestPlatformY = std::min(bestPlatformY, platform.y);
    }

    if (bestPlatformY == std::numeric_limits<float>::max()) {
        return std::nullopt;
    }

    return bestPlatformY;
}

[[nodiscard]] std::optional<float> FindLandingPlatformY(const SDL_FRect& playerRect,
                                                        float previousY,
                                                        std::span<const SDL_FRect> floorPlatforms) {
    const float previousBottom = previousY + playerRect.h;
    const float playerBottom = playerRect.y + playerRect.h;
    float bestPlatformY = std::numeric_limits<float>::max();

    for (const SDL_FRect& platform : floorPlatforms) {
        if (!HorizontallyOverlaps(playerRect, platform)) {
            continue;
        }

        if (previousBottom > platform.y || playerBottom < platform.y) {
            continue;
        }

        bestPlatformY = std::min(bestPlatformY, platform.y);
    }

    if (bestPlatformY == std::numeric_limits<float>::max()) {
        return std::nullopt;
    }

    return bestPlatformY;
}

}  // namespace

void ApplyPlayerCollisionProfile(PlayerCharacterState& player,
                                 const SpriteSheetFrame& frame,
                                 const PlayerControlConfig& config) {
    if (player.CollisionProfileInitialized) {
        return;
    }

    SOP_ASSERT(config.RenderScale > 0.0f, "Player collision profile requires a positive scale");
    SOP_ASSERT(frame.collision_box.w > 0.0f && frame.collision_box.h > 0.0f,
               "Player collision profile requires a positive collision box");

    const float previousBottom = player.CollisionRect.y + player.CollisionRect.h;
    const float anchorX = static_cast<float>(frame.anchor_x);
    const float anchorY = static_cast<float>(frame.anchor_y);
    const SDL_FRect collisionBox = frame.collision_box;

    player.CollisionRect.w = collisionBox.w * config.RenderScale;
    player.CollisionRect.h = collisionBox.h * config.RenderScale;
    player.CollisionRect.y = previousBottom - player.CollisionRect.h;
    player.CollisionAnchorOffset = SDL_FPoint{
        (anchorX - collisionBox.x) * config.RenderScale,
        (anchorY - collisionBox.y) * config.RenderScale,
    };
    player.FlippedCollisionAnchorOffset = SDL_FPoint{
        (collisionBox.x + collisionBox.w - anchorX) * config.RenderScale,
        player.CollisionAnchorOffset.y,
    };
    player.CollisionProfileInitialized = true;

    const SDL_FPoint offset = CollisionAnchorOffsetFor(player);
    player.AnchorPosition = SDL_FPoint{
        player.CollisionRect.x + offset.x,
        player.CollisionRect.y + offset.y,
    };
}

void ApplyBindings(PlayerInputState& input, const KeyEvent& event, const PlayerBindings& bindings) {
    if (event.Key == bindings.MoveLeft) {
        input.MoveLeft = event.Down;
    } else if (event.Key == bindings.MoveRight) {
        input.MoveRight = event.Down;
    } else if (event.Key == bindings.Jump) {
        if (event.Down && !event.Repeat) {
            input.JumpRequested = event.Down;
        }
    } else if (event.Key == bindings.Dash) {
        if (event.Down && !event.Repeat) {
            input.DashRequested = event.Down;
        }
    } else if (event.Key == bindings.Attack) {
        input.AttackHeld = event.Down;
    }
}

void SetPlayerSpawn(PlayerCharacterState& player, float posX, float posY, bool facingRight) {
    // init all values that may be useful on spawn / respawn
    player.FacingRight = facingRight;
    player.VerticalVelocity = 0.0f;
    player.Grounded = true;
    player.AirDashAvailable = true;
    player.AirJumpAvailable = false;
    player.DashSecondsRemaining = 0.0;
    player.DashCooldownSecondsRemaining = 0.0;
    player.DashDirection = facingRight ? 1.0f : -1.0f;

    player.CollisionRect.x = posX;
    player.CollisionRect.y = posY;
    const SDL_FPoint offset = CollisionAnchorOffsetFor(player);
    player.AnchorPosition =
        SDL_FPoint{player.CollisionRect.x + offset.x, player.CollisionRect.y + offset.y};
}

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                ParticleSystem& particleSystem,
                const PlayerControlConfig& config) {
    TickPlayer(player, input, stepSeconds, config, {}, particleSystem);
}

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config,
                std::span<const SDL_FRect> floorPlatforms,
                ParticleSystem& particleSystem) {
    const double elapsedSeconds = std::max(stepSeconds, 0.0);
    const float dt = static_cast<float>(elapsedSeconds);
    const bool attackActive = input.AttackHeld;
    float horizontalDirection = 0.0f;

    TickTimer(player.DashSecondsRemaining, elapsedSeconds);
    TickTimer(player.DashCooldownSecondsRemaining, elapsedSeconds);

    const bool canDash = player.Grounded || player.AirDashAvailable;
    if (input.DashRequested && !attackActive && !IsDashActive(player) &&
        player.DashCooldownSecondsRemaining <= 0.0 && canDash) {
        const bool airDash = !player.Grounded;
        player.DashDirection = SelectDashDirection(player, input);
        player.FacingRight = player.DashDirection > 0.0f;
        player.DashSecondsRemaining = std::max(config.DashSeconds, 0.0);
        player.DashCooldownSecondsRemaining = std::max(config.DashCooldownSeconds, 0.0);
        player.VerticalVelocity = 0.0f;

        if (airDash) {
            player.AirDashAvailable = false;
            player.AirJumpAvailable = true;
        }
    }
    input.DashRequested = false;

    if (IsDashActive(player)) {
        horizontalDirection = player.DashDirection;
    } else if (!attackActive && input.MoveLeft != input.MoveRight) {
        horizontalDirection = input.MoveLeft ? -1.0f : 1.0f;
        player.FacingRight = horizontalDirection > 0.0f;
    }

    const float horizontalSpeed = IsDashActive(player) ? config.DashSpeed : config.MoveSpeed;
    player.AnchorPosition.x += horizontalDirection * horizontalSpeed * dt;
    SyncCollisionRectToAnchor(player);
    SetPlayerCollisionX(player, std::clamp(player.CollisionRect.x, config.MinX, config.MaxX));

    ParticleBurstDesc hitEffect;
    hitEffect.Position = Vec2{player.AnchorPosition.x, player.AnchorPosition.y};
    hitEffect.Count = 24;
    hitEffect.MinSpeed = 120.0f;
    hitEffect.MaxSpeed = 320.0f;
    hitEffect.MinLifetime = 0.15f;
    hitEffect.MaxLifetime = 0.45f;
    hitEffect.MinSize = 4.0f;
    hitEffect.MaxSize = 10.0f;
    hitEffect.StartColor = Color{255, 220, 80, 255};
    hitEffect.EndColor = Color{255, 60, 40, 0};
    hitEffect.Acceleration = Vec2{0.0f, 500.0f};

    particleSystem.EmitBurst(hitEffect);

    if (input.JumpRequested && !IsDashActive(player)) {
        if (player.Grounded) {
            player.VerticalVelocity = config.JumpVelocity;
            player.Grounded = false;
            player.AirJumpAvailable = false;
        } else if (player.AirJumpAvailable) {
            player.VerticalVelocity = config.JumpVelocity;
            player.AirJumpAvailable = false;
        }
    }
    input.JumpRequested = false;

    if (IsDashActive(player)) {
        player.VerticalVelocity = 0.0f;
    } else if (floorPlatforms.empty()) {
        if (!player.Grounded) {
            player.VerticalVelocity += config.Gravity * dt;
            player.AnchorPosition.y += player.VerticalVelocity * dt;
            SyncCollisionRectToAnchor(player);

            if (player.CollisionRect.y >= config.GroundY) {
                LandPlayer(player, config.GroundY);
            }
        } else {
            LandPlayer(player, config.GroundY);
        }
    } else if (player.Grounded) {
        const std::optional<float> supportedPlatformY =
            FindSupportedPlatformY(player.CollisionRect, floorPlatforms);
        if (supportedPlatformY.has_value()) {
            LandPlayer(player, *supportedPlatformY - player.CollisionRect.h);
        } else {
            player.Grounded = false;
        }
    }

    if (!floorPlatforms.empty() && !player.Grounded && !IsDashActive(player)) {
        const float previousY = player.CollisionRect.y;
        player.VerticalVelocity += config.Gravity * dt;
        player.AnchorPosition.y += player.VerticalVelocity * dt;
        SyncCollisionRectToAnchor(player);

        const std::optional<float> landingPlatformY =
            FindLandingPlatformY(player.CollisionRect, previousY, floorPlatforms);
        if (landingPlatformY.has_value()) {
            LandPlayer(player, *landingPlatformY - player.CollisionRect.h);
        }
    }

    player.Animation.SetAnimation(SelectPlayerAnimation(player, input));
}

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         SDL_FPoint logicalViewportSize) {
    (void)logicalViewportSize;

    ApplyPlayerViewport(
        config, player, SDL_FRect{0.0f, 0.0f, kDefaultArenaWidth, kDefaultArenaHeight});
}

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         const SDL_FRect& arenaRect) {
    (void)arenaRect;

    config.MinX = 0.0f;
    config.MaxX = std::max(config.MinX, kDefaultArenaWidth - player.CollisionRect.w);
    config.GroundY = std::max(0.0f,
                              std::min(kDefaultArenaHeight - player.CollisionRect.h,
                                       kDefaultPlayerFloorLineY - player.CollisionRect.h));

    SyncCollisionRectToAnchor(player);
    SetPlayerCollisionX(player, std::clamp(player.CollisionRect.x, config.MinX, config.MaxX));
}

CharacterAnimation SelectPlayerAnimation(const PlayerCharacterState& player,
                                         const PlayerInputState& input) {
    if (IsDashActive(player)) {
        return CharacterAnimation::Dash;
    }

    if (input.AttackHeld) {
        return CharacterAnimation::Attacks;
    }

    if (!player.Grounded) {
        if (player.VerticalVelocity < 0.0f) {
            return CharacterAnimation::Ascending;
        }

        return CharacterAnimation::Falling;
    }

    if (input.MoveLeft != input.MoveRight) {
        return CharacterAnimation::Walk;
    }

    return CharacterAnimation::Idle;
}

}  // namespace sop
