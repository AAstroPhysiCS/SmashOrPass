#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

inline constexpr float kDefaultArenaWidth = 1280.0f;
inline constexpr float kDefaultArenaHeight = 720.0f;
inline constexpr float kArenaAspectRatio = kDefaultArenaWidth / kDefaultArenaHeight;

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

[[nodiscard]] inline SDL_FRect MapDesignRectToArena(const SDL_FRect& designRect,
                                                    const SDL_FRect& arenaRect,
                                                    SDL_FPoint designSize = SDL_FPoint{
                                                        kDefaultArenaWidth,
                                                        kDefaultArenaHeight,
                                                    }) {
    if (designSize.x <= 0.0f || designSize.y <= 0.0f || arenaRect.w <= 0.0f ||
        arenaRect.h <= 0.0f) {
        return SDL_FRect{};
    }

    const float scaleX = arenaRect.w / designSize.x;
    const float scaleY = arenaRect.h / designSize.y;

    return SDL_FRect{
        arenaRect.x + (designRect.x * scaleX),
        arenaRect.y + (designRect.y * scaleY),
        designRect.w * scaleX,
        designRect.h * scaleY,
    };
}

}  // namespace sop
