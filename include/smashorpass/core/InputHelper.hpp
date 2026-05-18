#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

#include <chrono>
#include <cstdint>
#include <glm/vec2.hpp>
#include <optional>
#include <unordered_map>

#include "smashorpass/core/Event.hpp"

namespace sop {

using Clock = std::chrono::steady_clock;

struct AppCtx;

struct KeyPressInfo {
    Clock::time_point PressedAt;
};

class InputHelper {
   private:
    std::unordered_map<SDL_Keycode, KeyPressInfo> m_KeysPressed;
    std::unordered_map<std::uint8_t, KeyPressInfo> m_MouseButtonsPressed;
    glm::vec2 m_CursorPosition{0.0f, 0.0f};

   public:
    void RecordEvent(AppCtx& ctx, const Event& event);

    [[nodiscard]] glm::vec2 GetCursorPosition() const noexcept {
        return m_CursorPosition;
    }

    [[nodiscard]] std::optional<KeyPressInfo> GetKeyPressInfo(SDL_Keycode key) const {
        const auto it = m_KeysPressed.find(key);
        if (it == m_KeysPressed.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] std::optional<KeyPressInfo> GetMouseButtonPressInfo(
        std::uint8_t mouse_button) const {
        const auto it = m_MouseButtonsPressed.find(mouse_button);
        if (it == m_MouseButtonsPressed.end()) {
            return std::nullopt;
        }
        return it->second;
    }
};

}  // namespace sop
