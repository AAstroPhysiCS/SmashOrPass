#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

#include "GameState.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>

namespace sop {

    struct KeyEvent {
        SDL_Keycode Key = 0;
        bool Down = false;
        bool Repeat = false;
    };

    struct MouseButtonEvent {
        uint8_t Button = 0;
        bool Down = false;
        float X = 0.0f;
        float Y = 0.0f;
    };

    struct WindowResizeEvent {
        int32_t Width = 0;
        int32_t Height = 0;
    };

    struct ControllerButtonEvent {
        uint8_t Button = 0;
        bool Down = false;
    };

    struct GameStateChangeEvent {
        GameState NextState = GameState::MainMenu;
    };

    using Event = std::variant<KeyEvent, MouseButtonEvent, WindowResizeEvent, ControllerButtonEvent, GameStateChangeEvent>;

    class EventDispatcher final {
    public:
        explicit EventDispatcher(const Event& event) 
            : m_Event(event) {}

        template <typename TEvent, typename TFunc>
        bool Dispatch(TFunc&& function) const {
            if (const auto* event = std::get_if<TEvent>(&m_Event)) {
                std::forward<TFunc>(function)(*event);
                return true;
            }
            return false;
        }
    private:
        const Event& m_Event;
    };

    inline static constexpr std::optional<Event> TranslateSDLEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                return KeyEvent{.Key = event.key.key, .Down = event.key.down, .Repeat = event.key.repeat};
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                return MouseButtonEvent{.Button = event.button.button, .Down = event.button.down, .X = event.button.x, .Y = event.button.y};
            case SDL_EVENT_WINDOW_RESIZED:
                return WindowResizeEvent{.Width = event.window.data1, .Height = event.window.data2};
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                return ControllerButtonEvent{.Button = event.gbutton.button, .Down = event.gbutton.down};
            default:
                return std::nullopt;
        }
    }
}