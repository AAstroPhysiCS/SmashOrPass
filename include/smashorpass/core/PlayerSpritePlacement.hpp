#pragma once

#include <SDL3/SDL_render.h>

#include "smashorpass/asset/SpriteSheet.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/Base.hpp"

namespace sop::detail {

struct PlayerSpritePlacement {
    SDL_FRect SourceRect{};
    SDL_FRect DestinationRect{};
    SDL_FPoint Origin{0.0f, 0.0f};
    SDL_FlipMode Flip{SDL_FLIP_NONE};
};

[[nodiscard]] Vec2 LocalFramePointToWorld(const PlayerCharacterState& player,
                                  const SpriteSheetFrame& frame,
                                  Vec2 localPoint,
                                  bool facingRight,
                                  float scale);

[[nodiscard]] PlayerSpritePlacement MakePlayerSpritePlacement(SDL_FPoint anchorPosition,
                                                              const SpriteSheetFrame& frame,
                                                              bool facingRight,
                                                              float scale);

}  // namespace sop::detail
