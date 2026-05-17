#include "smashorpass/app/Application.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <string>
#include <variant>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/state/GameState.hpp"
#include "smashorpass/state/UIState.hpp"

namespace sop {

Application::Application() {
    SOP_VERIFY(ctx.m_Renderer.SetVSync(true), "SDL_SetRenderVSync");

    auto displayMetricsResult = RefreshDisplayMetrics();
    if (!displayMetricsResult) {
        SOP_VERIFY(false, displayMetricsResult.error().c_str());
    }

    ctx.Assets =
        std::make_unique<AssetManager>(SOP_ASSET_ROOT_DIR, ctx.m_Renderer.NativeHandle());
}

Application::~Application() = default;

Result<void> Application::Run() {
    spdlog::info("Starting the game");

    while (ctx.AppRunning) {
        auto result = ProcessEvents();
        if (!result) {
            return result;
        }
        result = Update();
        if (!result) {
            return result;
        }
        result = Render();
        if (!result) {
            return result;
        }
    }

    spdlog::info("Shutting down the game");
    return Ok();
}

Result<void> Application::ProcessEvents() {
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (IsWindowMetricsEventType(event.type)) {
            auto result = RefreshDisplayMetrics();
            if (!result) {
                return result;
            }

            result = DispatchEvent(Event{
                .Payload = WindowMetricsChangedEvent{.Metrics = ctx.m_DisplayMetrics},
                .RawEvent = nullptr,
            });
            if (!result) {
                return result;
            }
        }

        SDL_Event translatedSource = event;
        if (IsPointerEventType(event.type)) {
            if (!ctx.m_Renderer.ConvertEventToRenderCoordinates(translatedSource)) {
                return Err(std::string("SDL_ConvertEventToRenderCoordinates failed: ") +
                           SDL_GetError());
            }
        }

        const Event translatedEvent = TranslateSDLEvent(translatedSource, &event);
        auto result = DispatchEvent(translatedEvent);
        if (!result) {
            return result;
        }

        if (event.type == SDL_EVENT_QUIT) {
            ctx.AppRunning = false;
        }
    }

    while (!ctx.m_EventDispatcher.m_EventQueue.empty()) {
        Event customEvent = std::move(ctx.m_EventDispatcher.m_EventQueue.front());
        ctx.m_EventDispatcher.m_EventQueue.pop_front();
        auto result = DispatchEvent(customEvent);
        if (!result) {
            return result;
        }
    }

    return Ok();
}

Result<void> Application::Update() {
    return ctx.m_StateManager.Update(ctx);
}

Result<void> Application::Render() {
    ctx.m_Renderer.BeginFrame();
    auto result = ctx.m_StateManager.Render(ctx);
    ctx.m_Renderer.EndFrame();
    return result;
}

Result<void> Application::DispatchEvent(const Event& event) {
    if (std::holds_alternative<ApplicationQuitEvent>(event.Payload) ||
        std::holds_alternative<ApplicationStateChangeEvent>(event.Payload)) {
        return OnEvent(event);
    }

    auto result = ctx.m_StateManager.DispatchEvent(ctx, event);
    if (!result) {
        return Err(result.error());
    }

    if (*result == EventFlow::Passed) {
        return OnEvent(event);
    }
    return Ok();
}

Result<void> Application::RefreshDisplayMetrics() {
    ctx.m_DisplayMetrics = ctx.m_Window.GetDisplayMetrics();

    const bool scaleApplied = ctx.m_Renderer.ApplyDisplayScale(ctx.m_DisplayMetrics.DisplayScale);
    if (!scaleApplied) {
        return Err(std::string("SDL_SetRenderScale failed: ") + SDL_GetError());
    }
    return Ok();
}

Result<void> Application::OnEvent(const Event& event) {
    if (std::get_if<ApplicationQuitEvent>(&event.Payload) != nullptr) {
        ctx.AppRunning = false;
        return Ok();
    }

    if (const auto* stateEvent = std::get_if<ApplicationStateChangeEvent>(&event.Payload)) {
        return Err(std::string("Application state changes are not implemented yet (requested: ") +
                   std::to_string(static_cast<int32_t>(stateEvent->NextState)) + ")");
    }

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && keyEvent->Key == SDLK_F1) {
            return ToggleDebugOverlay();
        }
    }

    return Ok();
}

Result<void> Application::ToggleDebugOverlay() {
    ctx.DebugOverlayVisible = !ctx.DebugOverlayVisible;
    spdlog::info("Debug overlay {}", ctx.DebugOverlayVisible ? "enabled" : "disabled");
    return Ok();
}

}  // namespace sop
