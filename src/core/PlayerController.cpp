#include "smashorpass/core/PlayerController.hpp"

#include <SDL3/SDL_keycode.h>

#include <algorithm>
#include <limits>
#include <optional>

namespace sop {
namespace {

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

    player.PlaceholderRect.x =
        std::clamp(player.PlaceholderRect.x + (horizontalDirection * config.MoveSpeed * dt),
                   config.MinX,
                   config.MaxX);

    if (input.JumpRequested && player.Grounded) {
        player.VerticalVelocity = config.JumpVelocity;
        player.Grounded = false;
    }
    input.JumpRequested = false;

    if (floorPlatforms.empty()) {
        if (!player.Grounded) {
            player.VerticalVelocity += config.Gravity * dt;
            player.PlaceholderRect.y += player.VerticalVelocity * dt;

            if (player.PlaceholderRect.y >= config.GroundY) {
                player.PlaceholderRect.y = config.GroundY;
                player.VerticalVelocity = 0.0f;
                player.Grounded = true;
            }
        } else {
            player.PlaceholderRect.y = config.GroundY;
            player.VerticalVelocity = 0.0f;
        }
    } else if (player.Grounded) {
        const std::optional<float> supportedPlatformY =
            FindSupportedPlatformY(player.PlaceholderRect, floorPlatforms);
        if (supportedPlatformY.has_value()) {
            player.PlaceholderRect.y = *supportedPlatformY - player.PlaceholderRect.h;
            player.VerticalVelocity = 0.0f;
        } else {
            player.Grounded = false;
        }
    }

    if (!floorPlatforms.empty() && !player.Grounded) {
        const float previousY = player.PlaceholderRect.y;
        player.VerticalVelocity += config.Gravity * dt;
        player.PlaceholderRect.y += player.VerticalVelocity * dt;

        const std::optional<float> landingPlatformY =
            FindLandingPlatformY(player.PlaceholderRect, previousY, floorPlatforms);
        if (landingPlatformY.has_value()) {
            player.PlaceholderRect.y = *landingPlatformY - player.PlaceholderRect.h;
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
    config.MaxX = std::max(config.MinX, kDefaultArenaWidth - player.PlaceholderRect.w);
    config.GroundY = std::max(0.0f,
                              std::min(kDefaultArenaHeight - player.PlaceholderRect.h,
                                       kDefaultPlayerFloorLineY - player.PlaceholderRect.h));

    player.PlaceholderRect.x = std::clamp(player.PlaceholderRect.x, config.MinX, config.MaxX);
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
