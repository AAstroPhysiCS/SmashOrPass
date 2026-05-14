#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

struct PushState {
    bool canPushLeft = true;
    bool canPushRight = true;
    bool canPushUp = true;
    bool canPushDown = false;
};

struct CollisionBody {
    SDL_FRect Rect{};
    PushState Push{};
    bool Dynamic = false;
    float Weight = 1;
};

}  // namespace sop
