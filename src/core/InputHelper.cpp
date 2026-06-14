#include "smashorpass/core/InputHelper.hpp"

#include <variant>

namespace sop {

void Input::RecordEvent(AppCtx& ctx, const Event& event) {
    (void)ctx;

    // Keyboard
    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat) {
            m_KeysPressed[keyEvent->Key] = KeyPressInfo{.PressedAt = Clock::now()};
        } else if (!keyEvent->Down) {
            m_KeysPressed.erase(keyEvent->Key);
        }
        return;
    }

    // Mouse button
    if (const auto* mouseButtonEvent = std::get_if<MouseButtonEvent>(&event.Payload)) {
        m_CursorPosition = glm::vec2{mouseButtonEvent->X, mouseButtonEvent->Y};
        if (mouseButtonEvent->Down) {
            m_MouseButtonsPressed[mouseButtonEvent->Button] =
                KeyPressInfo{.PressedAt = Clock::now()};
        } else {
            m_MouseButtonsPressed.erase(mouseButtonEvent->Button);
        }
        return;
    }

    // Cursor move
    if (const auto* mouseMovedEvent = std::get_if<MouseMovedEvent>(&event.Payload)) {
        m_CursorPosition = glm::vec2{mouseMovedEvent->X, mouseMovedEvent->Y};
    }
}

}  // namespace sop
