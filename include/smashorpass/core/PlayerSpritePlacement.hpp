#pragma once

#include <SDL3/SDL_render.h>

#include "smashorpass/asset/SpriteSheet.hpp"

namespace sop::detail {

struct PlayerSpritePlacement {
    SDL_FRect SourceRect{};
    SDL_FRect DestinationRect{};
    SDL_FPoint Origin{0.0f, 0.0f};
    SDL_FlipMode Flip{SDL_FLIP_NONE};
};

[[nodiscard]] PlayerSpritePlacement MakePlayerSpritePlacement(const SDL_FRect& placeholderRect,
                                                              const SpriteSheetFrame& frame,
                                                              bool facingRight,
                                                              float referenceSourceHeight);

}  // namespace sop::detail
