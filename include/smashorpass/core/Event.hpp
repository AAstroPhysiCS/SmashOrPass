#pragma once

#include <deque>

#include "Base.hpp"

#include "ApplicationState.hpp"

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

    struct MouseMovedEvent {
        float X = 0.0f;
        float Y = 0.0f;
        float XRel = 0.0f;
        float YRel = 0.0f;
    };

    struct WindowResizeEvent {
        int32_t Width = 0;
        int32_t Height = 0;
    };

    struct ControllerButtonEvent {
        uint8_t Button = 0;
        bool Down = false;
    };

    struct ApplicationStateChangeEvent {
        ApplicationState NextState = ApplicationState::MainMenu;
    };

    struct NullEvent {
    };

    using EventPayload = std::variant<KeyEvent, MouseButtonEvent, MouseMovedEvent, WindowResizeEvent, ControllerButtonEvent, ApplicationStateChangeEvent, NullEvent>;

    struct Event {
        EventPayload Payload;
        const SDL_Event* RawEvent = nullptr;
    };

    class EventDispatcher final {
    public:
        ~EventDispatcher() = default;

        template <typename TPayload>
        void Enqueue(TPayload&& payload) {
            // nullptr since this is a custom event, not directly from SDL
            m_EventQueue.emplace_back(payload, nullptr);
        }

        template <typename TEvent, typename TFunc>
        static bool Dispatch(const Event& evt, TFunc&& function) {
            if (const auto* event = std::get_if<TEvent>(&evt.Payload)) {
                std::forward<TFunc>(function)(*event);
                return true;
            }
            return false;
        }
    private:
        EventDispatcher() = default;
      
        // only application can administer the event queue, create event dispatcher etc... the ownership is application class only!
        std::deque<Event> m_EventQueue;

        friend class Application;
    };

    inline static constexpr Event TranslateSDLEvent(const SDL_Event& event) {
        Event result{};
        result.RawEvent = &event;

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                result.Payload = KeyEvent{.Key = event.key.key, .Down = event.key.down, .Repeat = event.key.repeat};
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                result.Payload = MouseButtonEvent{.Button = event.button.button, .Down = event.button.down, .X = event.button.x, .Y = event.button.y};
                break;
            case SDL_EVENT_MOUSE_MOTION:
                result.Payload = MouseMovedEvent{.X = event.motion.x, .Y = event.motion.y, .XRel = event.motion.xrel, .YRel = event.motion.yrel};
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                result.Payload = WindowResizeEvent{.Width = event.window.data1, .Height = event.window.data2};
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                result.Payload = ControllerButtonEvent{.Button = event.gbutton.button, .Down = event.gbutton.down};
                break;
            default:
                result.Payload = NullEvent{};
                break;
        }

        return result;
    }
}