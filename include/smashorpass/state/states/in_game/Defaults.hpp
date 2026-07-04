#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_rect.h>

#include <cstddef>
#include <string>

#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/core/Settings.hpp"
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

inline void FillInputTranslationFromBindings(InputTranslationHelper<InputAction>& helper,
                                             const PlayerKeyBindings& bindings) {
    helper.BindKey(bindings.MoveLeft, InputAction::MOVE_LEFT);
    helper.BindKey(bindings.MoveRight, InputAction::MOVE_RIGHT);
    helper.BindKey(bindings.Jump, InputAction::JUMP);
    helper.BindKey(bindings.Dash, InputAction::DASH);
    helper.BindKey(bindings.Attack, InputAction::ATTACK);
}

inline Result<void> FillDefaultInputTranslation(InputTranslationHelper<InputAction>& helper,
                                                const int player) {
    if (player == 0) {
        FillInputTranslationFromBindings(helper, Settings{}.PlayerKeyBindingsByPlayer[0]);
        return Ok();
    }

    if (player == 1) {
        FillInputTranslationFromBindings(helper, Settings{}.PlayerKeyBindingsByPlayer[1]);
        return Ok();
    }

    return Err(std::string{"Default Input Translation is not yet supported for player: "} +
               std::to_string(player));
}
}  // namespace sop
