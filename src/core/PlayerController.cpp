#include "smashorpass/core/PlayerController.hpp"

#include <SDL3/SDL_keycode.h>

#include <algorithm>

namespace sop {

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

    player.AttackSecondsRemaining = std::max(player.AttackSecondsRemaining - elapsedSeconds, 0.0);
    player.Animation.SetAnimation(SelectPlayerAnimation(player, input));
}

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         SDL_FPoint logicalViewportSize) {
    ApplyPlayerViewport(
        config, player, SDL_FRect{0.0f, 0.0f, logicalViewportSize.x, logicalViewportSize.y});
}

void ApplyPlayerViewport(PlayerControlConfig& config,
                         PlayerCharacterState& player,
                         const SDL_FRect& arenaRect) {
    const float arenaWidth = std::max(arenaRect.w, player.PlaceholderRect.w);
    const float arenaHeight = std::max(arenaRect.h, player.PlaceholderRect.h);
    const float arenaBottom = arenaRect.y + arenaHeight;
    const float floorLineY = arenaRect.y + (arenaHeight * kDefaultPlayerFloorLineRatio);

    config.MinX = arenaRect.x;
    config.MaxX = arenaRect.x + std::max(0.0f, arenaWidth - player.PlaceholderRect.w);
    config.GroundY = std::max(
        arenaRect.y,
        std::min(arenaBottom - player.PlaceholderRect.h, floorLineY - player.PlaceholderRect.h));

    player.PlaceholderRect.x = std::clamp(player.PlaceholderRect.x, config.MinX, config.MaxX);

    if (player.Grounded || player.PlaceholderRect.y > config.GroundY) {
        player.PlaceholderRect.y = config.GroundY;
        player.VerticalVelocity = 0.0f;
        player.Grounded = true;
    }
}

CharacterAnimation SelectPlayerAnimation(const PlayerCharacterState& player,
                                         const PlayerInputState& input) {
    if (player.AttackSecondsRemaining > 0.0) {
        return CharacterAnimation::Attacks;
    }

    if (!player.Grounded) {
        return CharacterAnimation::Jump;
    }

    if (input.MoveLeft != input.MoveRight) {
        return CharacterAnimation::Walk;
    }

    return CharacterAnimation::Idle;
}

}  // namespace sop
