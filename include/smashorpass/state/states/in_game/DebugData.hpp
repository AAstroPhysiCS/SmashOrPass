#pragma once

#include <SDL3/SDL_rect.h>

#include <optional>
#include <vector>

namespace sop {

struct PlayerCombatDebugData {
    std::optional<SDL_FRect> spriteRect;

    std::vector<SDL_FRect> hitBoxBounds;
    std::vector<SDL_FRect> hurtBoxBounds;
};

}  // namespace sop