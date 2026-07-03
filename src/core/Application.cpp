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
#include "smashorpass/state/states/match_results/MatchResultsState.hpp"

namespace sop {

Application::Application() {
    Result<void> success = Ok();

    TRY_AND_VOID(success, ctx.Initialize());
    TRY_AND_VOID(success, ctx.renderer.SetVSync(true));
    TRY_AND_VOID(success, RefreshDisplayMetrics());

    TRY_AND_VOID(success, ctx.stateManager.ResetToState<MainMenuState>(ctx));
    TRY_AND_VOID(success, ctx.stateManager.PushOverlay<DebugState>(ctx));

    if (success)
        m_Initialized = true;
}

Result<void> Application::Run() {
    if (!m_Initialized) {
        return Err(std::string("Application could not be initialized properly!"));
    }

    spdlog::info("Starting the game");

    while (ctx.appRunning) {
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
                .Payload = WindowMetricsChangedEvent{.Metrics = ctx.displayMetrics},
                .RawEvent = nullptr,
            }));
        }

        SDL_Event translatedSource = event;
        if (IsPointerEventType(event.type)) {
            TRY_VOID(ctx.renderer.ConvertEventToRenderCoordinates(translatedSource));
        }

        const Event translatedEvent = TranslateSDLEvent(translatedSource);
        TRY_VOID(DispatchEvent(translatedEvent));

        if (event.type == SDL_EVENT_QUIT) {
            ctx.appRunning = false;
        }
    }

    while (!ctx.eventDispatcher.m_EventQueue.empty()) {
        Event customEvent = std::move(ctx.eventDispatcher.m_EventQueue.front());
        ctx.eventDispatcher.m_EventQueue.pop_front();
        TRY_VOID(DispatchEvent(customEvent));
    }

    return Ok();
}

Result<void> Application::Update() {
    ctx.assets.Update();
    return ctx.stateManager.Update(ctx);
}

Result<void> Application::Render() {
    TRY_VOID(ctx.renderer.BeginFrame());
    auto result = ctx.stateManager.Render(ctx);
    TRY_VOID(ctx.renderer.EndFrame());
    return result;
}

Result<void> Application::DispatchEvent(const Event& event) {
    ctx.input.RecordEvent(ctx, event);

    if (std::holds_alternative<ApplicationQuitEvent>(event.Payload)) {
        return OnEvent(event);
    }

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat && keyEvent->Key == SDLK_F1) {
            return OnEvent(event);
        }
    }

    TRY(eventFlow, ctx.stateManager.DispatchEvent(ctx, event));
    if (eventFlow == EventFlow::Passed) {
        return OnEvent(event);
    }
    return Ok();
}

Result<void> Application::RefreshDisplayMetrics() {
    TRY(displayMetrics, ctx.window.GetDisplayMetrics());
    ctx.displayMetrics = displayMetrics;

    return ctx.renderer.ApplyDisplayScale(ctx.displayMetrics.DisplayScale);
}

Result<void> Application::OnEvent(const Event& event) {
    {
        TRY(handled,
            EventDispatcher::Dispatch<ApplicationQuitEvent>(
                event, [&](const ApplicationQuitEvent&) { ctx.appRunning = false; }));

        if (handled) {
            return Ok();
        }
    }

    {
        TRY(handled,
            EventDispatcher::Dispatch<NavigationEvent>(
                event, [&](const NavigationEvent& navigation) -> Result<void> {
                    switch (navigation.Action) {
                        case NavigationAction::ShowMainMenu: {
                            ctx.particleSystem.Clear();

                            TRY(mainMenuState, ctx.stateManager.ResetToState<MainMenuState>(ctx));
                            (void)mainMenuState;

                            return Ok();
                        }
                        case NavigationAction::StartMatch: {
                            ctx.particleSystem.Clear();

                            TRY(inGameState,
                                ctx.stateManager.ResetToState<InGameState>(
                                    ctx, navigation.Match));
                            (void)inGameState;

                            return Ok();
                        }
                        case NavigationAction::ShowMatchResults: {
                            ctx.particleSystem.Clear();
                            ctx.overallStats.RecordMatch(navigation.Match, navigation.Results);

                            TRY(matchResultsState,
                                ctx.stateManager.ResetToState<MatchResultsState>(
                                    ctx, navigation.Match, navigation.Results));
                            (void)matchResultsState;

                            return Ok();
                        }
                        case NavigationAction::ShowGameModeSelect:
                        case NavigationAction::ShowCharacterSelect:
                        case NavigationAction::ShowScoreboard:
                        case NavigationAction::ResumeMatch:
                            return Ok();
                    }
                    return Ok();
                }));

        if (handled) {
            return Ok();
        }
    }

    {
        TRY(handled,
            EventDispatcher::Dispatch<KeyEvent>(
                event, [&](const KeyEvent& keyEvent) -> Result<void> {
                    if (keyEvent.Down && !keyEvent.Repeat && keyEvent.Key == SDLK_F1) {
                        TRY_VOID(ToggleDebugOverlay());
                    }

                    return Ok();
                }));

        if (handled) {
            return Ok();
        }
    }

    return Ok();
}

Result<void> Application::ToggleDebugOverlay() {
    ctx.debugOverlayVisible = !ctx.debugOverlayVisible;
    spdlog::info("Debug overlay {}", ctx.debugOverlayVisible ? "enabled" : "disabled");
    return Ok();
}

}  // namespace sop
