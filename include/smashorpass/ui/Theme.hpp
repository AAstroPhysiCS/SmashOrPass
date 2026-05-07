#pragma once

#include <cmath>
#include <algorithm>

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

    static float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    static Color LerpColor(Color a, Color b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);

        return Color{
            static_cast<uint8_t>(Lerp(static_cast<float>(a.r), static_cast<float>(b.r), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.g), static_cast<float>(b.g), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.b), static_cast<float>(b.b), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.a), static_cast<float>(b.a), t)),
        };
    }

    namespace Theme {

        // General UI
        static inline constexpr Color SCREEN_BACKGROUND_COLOR{18, 20, 26, 255};
        static inline constexpr Color PANEL_BACKGROUND_COLOR{28, 32, 40, 230};
        static inline constexpr Color PANEL_BORDER_COLOR{90, 100, 120, 255};
        
        // Text
        static inline constexpr Color TEXT_PRIMARY_COLOR{235, 235, 235, 255};
        static inline constexpr Color TEXT_SECONDARY_COLOR{170, 175, 185, 255};
        static inline constexpr Color TEXT_DISABLED_COLOR{100, 105, 115, 255};
        
        // Buttons
        static inline constexpr Color BUTTON_BACKGROUND_COLOR{52, 58, 64, 255};
        static inline constexpr Color BUTTON_HOVER_BACKGROUND_COLOR{70, 78, 88, 255};
        static inline constexpr Color BUTTON_PRESSED_BACKGROUND_COLOR{35, 40, 48, 255};
        static inline constexpr Color BUTTON_BORDER_COLOR{90, 100, 110, 255};
        static inline constexpr Color BUTTON_TEXT_COLOR{255, 255, 255, 255};
        
        static inline constexpr float BUTTON_PADDING_X = 14.0f;
        static inline constexpr float BUTTON_PADDING_Y = 10.0f;
        
        // Menu-specific buttons
        static inline constexpr Color MENU_BUTTON_COLOR{52, 58, 64, 255};
        static inline constexpr Color MENU_BUTTON_HOVER_COLOR{75, 85, 100, 255};
        static inline constexpr Color MENU_BUTTON_BORDER_COLOR{110, 125, 145, 255};
        static inline constexpr Color MENU_BUTTON_TEXT_COLOR{255, 255, 255, 255};
        
        // Player colors
        static inline constexpr Color PLAYER_1_COLOR{255, 70, 70, 255};
        static inline constexpr Color PLAYER_2_COLOR{70, 130, 255, 255};
        
        // Character select
        static inline constexpr Color CHARACTER_AVAILABLE_COLOR{52, 58, 64, 255};
        static inline constexpr Color CHARACTER_SELECTED_P1_COLOR{120, 40, 40, 255};
        static inline constexpr Color CHARACTER_SELECTED_P2_COLOR{35, 60, 140, 255};
        static inline constexpr Color CHARACTER_SELECTED_BOTH_COLOR{110, 70, 160, 255};
        
        // Feedback colors
        static inline constexpr Color SUCCESS_COLOR{80, 220, 120, 255};
        static inline constexpr Color WARNING_COLOR{255, 190, 80, 255};
        static inline constexpr Color ERROR_COLOR{255, 60, 60, 255};

    }  // namespace Theme
}

