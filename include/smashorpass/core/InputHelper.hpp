#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

#include <chrono>
#include <cstdint>
#include <glm/vec2.hpp>
#include <optional>
#include <unordered_map>
#include <unordered_set>  // added

#include "smashorpass/core/Event.hpp"

namespace sop {

using Clock = std::chrono::steady_clock;

struct AppCtx;

struct KeyPressInfo {
    Clock::time_point PressedAt;
};

// ---- GENERAL PURPOSE INPUT HELPER --------------------------
// This is part of the AppCtx and is available to all states.

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

// ---- INPUT TRANSLATION HELPER ------------------------------
// This is part of a state if that state needs it.

template <typename Action>
class InputTranslationHelper {
   private:
    std::unordered_map<SDL_Keycode, Action> m_KeyboardMapping;
    std::unordered_map<Action, std::unordered_set<SDL_Keycode>> m_ActionKeyboardMapping;

   public:
    void BindKey(SDL_Keycode key, Action action) {
        const auto existing = m_KeyboardMapping.find(key);

        if (existing != m_KeyboardMapping.end()) {
            auto reverse_it = m_ActionKeyboardMapping.find(existing->second);
            if (reverse_it != m_ActionKeyboardMapping.end()) {
                reverse_it->second.erase(key);

                if (reverse_it->second.empty()) {
                    m_ActionKeyboardMapping.erase(reverse_it);
                }
            }

            existing->second = action;
        } else {
            m_KeyboardMapping.emplace(key, action);
        }

        m_ActionKeyboardMapping[action].insert(key);
    }

    [[nodiscard]] std::optional<Action> TranslateKey(SDL_Keycode key) const {
        const auto it = m_KeyboardMapping.find(key);
        if (it == m_KeyboardMapping.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] const std::unordered_set<SDL_Keycode>* GetKeysForAction(
        const Action& action) const {
        const auto it = m_ActionKeyboardMapping.find(action);
        if (it == m_ActionKeyboardMapping.end()) {
            return nullptr;
        }
        return &it->second;
    }
};

}  // namespace sop
