#include "smashorpass/core/Application.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <variant>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/state/overlays/DebugState.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"
#include "smashorpass/state/states/main_menu/MainMenuState.hpp"

namespace sop {

Application::Application() {
    SOP_VERIFY(ctx.m_Renderer.SetVSync(true), "SDL_SetRenderVSync");

    auto displayMetricsResult = RefreshDisplayMetrics();
    if (!displayMetricsResult) {
        SOP_VERIFY(false, displayMetricsResult.error().c_str());
    }

    ctx.Assets = std::make_unique<AssetManager>(SOP_ASSET_ROOT_DIR, ctx.m_Renderer.NativeHandle());
    ctx.m_StateManager.ResetToState<MainMenuState>(ctx);
    ctx.m_StateManager.PushOverlay<DebugState>(ctx);
}

Application::~Application() = default;

Result<void> Application::Run() {
    spdlog::info("Starting the game");

    while (ctx.AppRunning) {
        TRY_VOID(ProcessEvents());
        TRY_VOID(Update());
        TRY_VOID(Render());
    }

    spdlog::info("Shutting down the game");
    return Ok();
}

Result<void> Application::ProcessEvents() {
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (IsWindowMetricsEventType(event.type)) {
            TRY_VOID(RefreshDisplayMetrics());

            TRY_VOID(DispatchEvent(Event{
                .Payload = WindowMetricsChangedEvent{.Metrics = ctx.m_DisplayMetrics},
                .RawEvent = nullptr,
            }));
        }

        SDL_Event translatedSource = event;
        if (IsPointerEventType(event.type)) {
            if (!ctx.m_Renderer.ConvertEventToRenderCoordinates(translatedSource)) {
                return Err(std::string("SDL_ConvertEventToRenderCoordinates failed: ") +
                           SDL_GetError());
            }
        }

        const Event translatedEvent = TranslateSDLEvent(translatedSource, &event);
        TRY_VOID(DispatchEvent(translatedEvent));

        if (event.type == SDL_EVENT_QUIT) {
            ctx.AppRunning = false;
        }
    }

    while (!ctx.m_EventDispatcher.m_EventQueue.empty()) {
        Event customEvent = std::move(ctx.m_EventDispatcher.m_EventQueue.front());
        ctx.m_EventDispatcher.m_EventQueue.pop_front();
        TRY_VOID(DispatchEvent(customEvent));
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
    if (std::holds_alternative<ApplicationQuitEvent>(event.Payload)) {
        return OnEvent(event);
    }

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat && keyEvent->Key == SDLK_F1) {
            return OnEvent(event);
        }
    }

    TRY(eventFlow, ctx.m_StateManager.DispatchEvent(ctx, event));
    if (eventFlow == EventFlow::Passed) {
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

    if (const auto* navigation = std::get_if<NavigationEvent>(&event.Payload)) {
        switch (navigation->Action) {
            case NavigationAction::ShowMainMenu:
                ctx.m_ParticleSystem.Clear();
                ctx.m_StateManager.ResetToState<MainMenuState>(ctx);
                return Ok();
            case NavigationAction::StartMatch:
                if (ctx.Assets != nullptr) {
                    (void)ctx.Assets->getArenaBackgroundTexture(ArenaId::Chains);
                    (void)ctx.Assets->getArenaForegroundTexture(ArenaId::Chains);
                    ctx.Assets->preloadCharacterSpriteSheets(kDefaultCharacterId);
                }
                ctx.m_ParticleSystem.Clear();
                ctx.m_StateManager.ResetToState<InGameState>(ctx);
                return Ok();
            case NavigationAction::ShowCharacterSelect:
            case NavigationAction::ResumeMatch:
                return Ok();
        }
    }

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat && keyEvent->Key == SDLK_F1) {
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
