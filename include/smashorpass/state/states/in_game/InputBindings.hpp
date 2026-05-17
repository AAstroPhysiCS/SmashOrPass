#pragma once

#include <SDL3/SDL_keycode.h>

namespace sop {

struct PlayerBindings {
    SDL_Keycode MoveLeft;
    SDL_Keycode MoveRight;
    SDL_Keycode Jump;
    SDL_Keycode Dash;
    SDL_Keycode Attack;
};

}  // namespace sop