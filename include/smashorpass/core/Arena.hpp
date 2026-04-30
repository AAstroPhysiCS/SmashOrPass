#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

inline constexpr float kArenaAspectRatio = 16.0f / 9.0f;

[[nodiscard]] inline SDL_FRect MakeContainedArenaRect(SDL_FPoint logicalSize,
                                                      float aspectRatio = kArenaAspectRatio) {
    if (logicalSize.x <= 0.0f || logicalSize.y <= 0.0f || aspectRatio <= 0.0f) {
        return SDL_FRect{};
    }

    float arenaWidth = logicalSize.x;
    float arenaHeight = arenaWidth / aspectRatio;

    if (arenaHeight > logicalSize.y) {
        arenaHeight = logicalSize.y;
        arenaWidth = arenaHeight * aspectRatio;
    }

    return SDL_FRect{(logicalSize.x - arenaWidth) * 0.5f,
                     (logicalSize.y - arenaHeight) * 0.5f,
                     arenaWidth,
                     arenaHeight};
}

}  // namespace sop
