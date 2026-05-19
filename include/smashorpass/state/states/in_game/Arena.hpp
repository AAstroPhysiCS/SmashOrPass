#pragma once

#include <SDL3/SDL_rect.h>

#include "smashorpass/asset/AssetManager.hpp"

namespace sop {

struct Arena {
    ArenaAssetHandle asset;
    // Rectangular space in the window the arena actually occupies.
    SDL_Rect dimensions{};

    void ResizeToWindow(SDL_FPoint logicalWindowSize) {
        if (logicalWindowSize.x <= 0.0f || logicalWindowSize.y <= 0.0f) {
            return;
        }

        float arenaWidth = logicalWindowSize.x;
        float arenaHeight = arenaWidth / ARENA_BASELINE_ASPECT_RATIO;

        if (arenaHeight > logicalWindowSize.y) {
            arenaHeight = logicalWindowSize.y;
            arenaWidth = arenaHeight * ARENA_BASELINE_ASPECT_RATIO;
        }

        dimensions = SDL_Rect{
            static_cast<int>((logicalWindowSize.x - arenaWidth) * 0.5f),
            static_cast<int>((logicalWindowSize.y - arenaHeight) * 0.5f),
            static_cast<int>(arenaWidth),
            static_cast<int>(arenaHeight),
        };
    }
};

[[nodiscard]] inline SDL_FRect MapBaselineRectToArena(const SDL_FRect& baselineRect,
                                                      const SDL_Rect& arenaRect) {
    if (arenaRect.w <= 0 || arenaRect.h <= 0) {
        return SDL_FRect{};
    }

    const float scaleX = static_cast<float>(arenaRect.w) / static_cast<float>(ARENA_BASELINE_WIDTH);
    const float scaleY =
        static_cast<float>(arenaRect.h) / static_cast<float>(ARENA_BASELINE_HEIGHT);

    return SDL_FRect{
        static_cast<float>(arenaRect.x) + baselineRect.x * scaleX,
        static_cast<float>(arenaRect.y) + baselineRect.y * scaleY,
        baselineRect.w * scaleX,
        baselineRect.h * scaleY,
    };
}

}  // namespace sop
