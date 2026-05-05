#include "smashorpass/core/PlayerController.hpp"

#include <SDL3/SDL_keycode.h>

#include <algorithm>
#include <limits>
#include <optional>

#include "smashorpass/core/Base.hpp"

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

void ApplyPlayerKeyEvent(PlayerInputState& input,
                         PlayerCharacterState& player,
                         const KeyEvent& event,
                         const PlayerControlConfig& config) {
    switch (event.Key) {
        case SDLK_A:
            input.MoveLeft = event.Down;
            break;
        case SDLK_D:
            input.MoveRight = event.Down;
            break;
        case SDLK_W:
            if (event.Down && !event.Repeat) {
                input.JumpRequested = true;
            }
            break;
        case SDLK_S:
            break;
        case SDLK_SPACE:
            if (event.Down && !event.Repeat) {
                player.AttackSecondsRemaining = config.AttackSeconds;
            }
            break;
        default:
            break;
    }
}

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config) {
    TickPlayer(player, input, stepSeconds, config, {});
}

void TickPlayer(PlayerCharacterState& player,
                PlayerInputState& input,
                double stepSeconds,
                const PlayerControlConfig& config,
                std::span<const SDL_FRect> floorPlatforms) {
    const double elapsedSeconds = std::max(stepSeconds, 0.0);
    const float dt = static_cast<float>(elapsedSeconds);
    float horizontalDirection = 0.0f;

    if (input.MoveLeft != input.MoveRight) {
        horizontalDirection = input.MoveLeft ? -1.0f : 1.0f;
        player.FacingRight = horizontalDirection > 0.0f;
    }

    player.AnchorPosition.x += horizontalDirection * config.MoveSpeed * dt;
    SyncCollisionRectToAnchor(player);
    SetPlayerCollisionX(player, std::clamp(player.CollisionRect.x, config.MinX, config.MaxX));

    if (input.JumpRequested && player.Grounded) {
        player.VerticalVelocity = config.JumpVelocity;
        player.Grounded = false;
    }
    input.JumpRequested = false;

    if (floorPlatforms.empty()) {
        if (!player.Grounded) {
            player.VerticalVelocity += config.Gravity * dt;
            player.AnchorPosition.y += player.VerticalVelocity * dt;
            SyncCollisionRectToAnchor(player);

            if (player.CollisionRect.y >= config.GroundY) {
                SetPlayerCollisionY(player, config.GroundY);
                player.VerticalVelocity = 0.0f;
                player.Grounded = true;
            }
        } else {
            SetPlayerCollisionY(player, config.GroundY);
            player.VerticalVelocity = 0.0f;
        }
    } else if (player.Grounded) {
        const std::optional<float> supportedPlatformY =
            FindSupportedPlatformY(player.CollisionRect, floorPlatforms);
        if (supportedPlatformY.has_value()) {
            SetPlayerCollisionY(player, *supportedPlatformY - player.CollisionRect.h);
            player.VerticalVelocity = 0.0f;
        } else {
            player.Grounded = false;
        }
    }

    if (!floorPlatforms.empty() && !player.Grounded) {
        const float previousY = player.CollisionRect.y;
        player.VerticalVelocity += config.Gravity * dt;
        player.AnchorPosition.y += player.VerticalVelocity * dt;
        SyncCollisionRectToAnchor(player);

        const std::optional<float> landingPlatformY =
            FindLandingPlatformY(player.CollisionRect, previousY, floorPlatforms);
        if (landingPlatformY.has_value()) {
            SetPlayerCollisionY(player, *landingPlatformY - player.CollisionRect.h);
            player.VerticalVelocity = 0.0f;
            player.Grounded = true;
        }
    }

    player.AttackSecondsRemaining = std::max(player.AttackSecondsRemaining - elapsedSeconds, 0.0);
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
    if (player.AttackSecondsRemaining > 0.0) {
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
