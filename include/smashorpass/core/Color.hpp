#pragma once

#include <SDL3/SDL_pixels.h>

#include <algorithm>
#include <cstdint>

namespace sop {

struct Color {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    uint8_t a{255};

    static constexpr Color White() {
        return {255, 255, 255, 255};
    }

    static constexpr Color Black() {
        return {0, 0, 0, 255};
    }

    static constexpr Color Transparent() {
        return {0, 0, 0, 0};
    }

    inline operator SDL_Color() {
        return SDL_Color{r, g, b, a};
    }
};

[[nodiscard]] inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

[[nodiscard]] inline Color LerpColor(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    return Color{
        static_cast<uint8_t>(Lerp(static_cast<float>(a.r), static_cast<float>(b.r), t)),
        static_cast<uint8_t>(Lerp(static_cast<float>(a.g), static_cast<float>(b.g), t)),
        static_cast<uint8_t>(Lerp(static_cast<float>(a.b), static_cast<float>(b.b), t)),
        static_cast<uint8_t>(a.a),
    };
}

}  // namespace sop
