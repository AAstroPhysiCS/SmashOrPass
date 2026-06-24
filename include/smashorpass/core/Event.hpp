#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>

#include <deque>
#include <vector>

#include "Base.hpp"
#include "DisplayMetrics.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/ArenaAsset.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/state/states/in_game/GameMode.hpp"
#include "smashorpass/state/states/in_game/PlayerControl.hpp"

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

struct WindowMetricsChangedEvent {
    DisplayMetrics Metrics{};
};

struct ControllerButtonEvent {
    uint8_t Button = 0;
    bool Down = false;
};

enum class NavigationAction {
    ShowMainMenu,
    ShowGameModeSelect,
    ShowCharacterSelect,
    StartMatch,
    ResumeMatch,
};

struct NavigationEvent {
    NavigationAction Action = NavigationAction::ShowMainMenu;
    GameMode Mode = GameMode::Smash;
    Asset<ArenaAssetData> ArenaAsset{};
    std::vector<Asset<CharacterAssetData>> CharacterAssets{};
    std::vector<PlayerControl> PlayerControls{};
};

struct ApplicationQuitEvent {};

enum class PlayerParticleEffectType {
    SwordFire,
    DashBlue,
};

struct PlayerParticleEffectEvent {
    PlayerParticleEffectType Type = PlayerParticleEffectType::SwordFire;
    Vec2 Position{};
    Vec2 Velocity{};
    bool FacingRight = false;
    float Strength = 1.0f;
};

struct NullEvent {};

enum class EventFlow {
    Passed,
    Consumed,
};

using EventPayload = std::variant<KeyEvent,
                                  MouseButtonEvent,
                                  MouseMovedEvent,
                                  WindowResizeEvent,
                                  WindowMetricsChangedEvent,
                                  ControllerButtonEvent,
                                  NavigationEvent,
                                  ApplicationQuitEvent,
                                  PlayerParticleEffectEvent,
                                  NullEvent>;

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
    static Result<bool> Dispatch(const Event& evt, TFunc&& function) {
        const auto* event = std::get_if<TEvent>(&evt.Payload);
        if (event == nullptr)
            return Ok(false);

        using ReturnType = std::invoke_result_t<TFunc, const TEvent&>;

        if constexpr (std::is_void_v<ReturnType>) {
            std::invoke(std::forward<TFunc>(function), *event);
            return Ok(true);
        } else {
            auto result = std::invoke(std::forward<TFunc>(function), *event);
            if (!result)
                return Err(std::move(result).error());
            return Ok(true);
        }
    }

   private:
    EventDispatcher() = default;

    std::deque<Event> m_EventQueue;

    friend struct AppCtx;
    friend class Application;
};

[[nodiscard]] inline constexpr bool IsWindowMetricsEventType(Uint32 type) {
    switch (type) {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] inline constexpr bool IsPointerEventType(Uint32 type) {
    switch (type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_MOTION:
            return true;
        default:
            return false;
    }
}

inline static Event TranslateSDLEvent(const SDL_Event& event) {
    Event result{};
    result.RawEvent = &event;

    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            result.Payload =
                KeyEvent{.Key = event.key.key, .Down = event.key.down, .Repeat = event.key.repeat};
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            result.Payload = MouseButtonEvent{.Button = event.button.button,
                                              .Down = event.button.down,
                                              .X = event.button.x,
                                              .Y = event.button.y};
            break;
        case SDL_EVENT_MOUSE_MOTION:
            result.Payload = MouseMovedEvent{.X = event.motion.x,
                                             .Y = event.motion.y,
                                             .XRel = event.motion.xrel,
                                             .YRel = event.motion.yrel};
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            result.Payload =
                WindowResizeEvent{.Width = event.window.data1, .Height = event.window.data2};
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            result.Payload =
                ControllerButtonEvent{.Button = event.gbutton.button, .Down = event.gbutton.down};
            break;
        default:
            result.Payload = NullEvent{};
            break;
    }

    return result;
}
}  // namespace sop
