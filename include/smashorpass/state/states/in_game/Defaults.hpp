#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_rect.h>

#include <cstddef>
#include <string>

#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"
#include "smashorpass/util.hpp"

namespace sop {

inline constexpr float kDefaultPlayerHealth = 100.0f;
inline constexpr int kDefaultPlayerStocks = 3;
inline constexpr float kBottomBlastZonePadding = 150.0f;
inline constexpr float kRespawnHeightAboveArena = 100.0f;
inline constexpr float kPlayerScale = 0.4f;

inline SDL_FPoint PlayerStartPosition(const std::size_t playerIndex) {
    switch (playerIndex) {
        case 0:
            return SDL_FPoint{508.0f, 300.0f};
        case 1:
            return SDL_FPoint{1412.0f, 300.0f};
        case 2:
            return SDL_FPoint{760.0f, 300.0f};
        case 3:
            return SDL_FPoint{1160.0f, 300.0f};
        default:
            return SDL_FPoint{960.0f, 300.0f};
    }
}

inline Result<void> FillDefaultInputTranslation(InputTranslationHelper<InputAction>& helper,
                                                const int player) {
    if (player == 0) {
        // Player 1: WASD
        helper.BindKey(SDLK_A, InputAction::MOVE_LEFT);
        helper.BindKey(SDLK_D, InputAction::MOVE_RIGHT);
        helper.BindKey(SDLK_W, InputAction::JUMP);
        helper.BindKey(SDLK_LSHIFT, InputAction::DASH);
        helper.BindKey(SDLK_SPACE, InputAction::ATTACK);
        return Ok();
    }

    if (player == 1) {
        // Player 2: arrows
        helper.BindKey(SDLK_LEFT, InputAction::MOVE_LEFT);
        helper.BindKey(SDLK_RIGHT, InputAction::MOVE_RIGHT);
        helper.BindKey(SDLK_UP, InputAction::JUMP);
        helper.BindKey(SDLK_RSHIFT, InputAction::DASH);
        helper.BindKey(SDLK_RCTRL, InputAction::ATTACK);
        return Ok();
    }

    return Err(std::string{"Default Input Translation is not yet supported for player: "} +
               std::to_string(player));
}
}  // namespace sop
