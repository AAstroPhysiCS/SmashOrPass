#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

inline constexpr float kDefaultDisplayScale = 1.0f;

[[nodiscard]] inline constexpr float NormalizeDisplayScale(float scale) {
    return scale > 0.0f ? scale : kDefaultDisplayScale;
}

struct DisplayMetrics {
    SDL_Point WindowSize{0, 0};
    SDL_Point PixelSize{0, 0};
    float DisplayScale{kDefaultDisplayScale};
    float PixelDensity{kDefaultDisplayScale};

    [[nodiscard]] SDL_FPoint LogicalSize() const {
        const float scale = NormalizeDisplayScale(DisplayScale);
        return SDL_FPoint{static_cast<float>(PixelSize.x) / scale,
                          static_cast<float>(PixelSize.y) / scale};
    }
};

}  // namespace sop
