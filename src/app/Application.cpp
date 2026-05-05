#include "smashorpass/app/Application.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include <chrono>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/layer/DebugLayer.hpp"
#include "smashorpass/layer/GameLayer.hpp"
#include "smashorpass/layer/UILayer.hpp"

namespace sop {
Application::Application()
    : m_Window(
          WindowCreateInfo{.Width = 1920, .Height = 1080, .Title = "Smash Or Pass - The Game"}),
      m_Renderer(m_Window) {
    RefreshDisplayMetrics();
    m_Context.Assets =
        std::make_unique<AssetManager>(SOP_ASSET_ROOT_DIR, m_Renderer.NativeHandle());
    ChangeState(ApplicationState::MainMenu);
}

Application::~Application() {
    while (!m_Overlays.empty()) {
        m_Overlays.pop_back();
    }
}

int Application::Run() {
    using Clock = std::chrono::steady_clock;

    bool running = true;
    Clock::time_point previousFrameTime = Clock::now();
    spdlog::info("Starting the game");

    while (running) {
        ProcessEvents(running);
        const Clock::time_point currentFrameTime = Clock::now();
        const auto elapsed = std::chrono::duration_cast<FixedStepScheduler::Duration>(
            currentFrameTime - previousFrameTime);
        previousFrameTime = currentFrameTime;

        if (m_Context.CurrentState == ApplicationState::Playing) {
            TickGameplay(elapsed);
            TickAnimation(elapsed);
        }

        Update();
        Render();
    }

    spdlog::info("Shutting down the game");
    return 0;
}

void Application::ProcessEvents(bool& running) {
    // SDL events
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
        if (IsWindowMetricsEventType(event.type)) {
            RefreshDisplayMetrics();
            DispatchEvent(Event{
                .Payload = WindowMetricsChangedEvent{.Metrics = m_Context.Display},
                .RawEvent = nullptr,
            });
        }

        SDL_Event translatedSource = event;
        if (IsPointerEventType(event.type)) {
            (void)m_Renderer.ConvertEventToRenderCoordinates(translatedSource);
        }

        const Event translatedEvent = TranslateSDLEvent(translatedSource, &event);
        DispatchEvent(translatedEvent);

        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }

    // Custom events
    while (!m_EventDispatcher.m_EventQueue.empty()) {
        Event customEvent = std::move(m_EventDispatcher.m_EventQueue.front());
        m_EventDispatcher.m_EventQueue.pop_front();
        DispatchEvent(customEvent);
    }
}

void Application::TickGameplay(FixedStepScheduler::Duration elapsed) {
    const uint32_t ticksDue = m_GameplayScheduler.Advance(elapsed);
    const uint64_t tickBase = m_GameplayScheduler.GetTotalTicks() - static_cast<uint64_t>(ticksDue);

    for (uint32_t tickIndex = 0; tickIndex < ticksDue; ++tickIndex) {
        m_Context.GameplayTickCount = tickBase + static_cast<uint64_t>(tickIndex) + 1ULL;
        m_CurrentLayer->OnGameplayTick(m_Context);
    }
}

void Application::TickAnimation(FixedStepScheduler::Duration elapsed) {
    const uint32_t ticksDue = m_AnimationScheduler.Advance(elapsed);
    const uint64_t tickBase =
        m_AnimationScheduler.GetTotalTicks() - static_cast<uint64_t>(ticksDue);

    for (uint32_t tickIndex = 0; tickIndex < ticksDue; ++tickIndex) {
        m_Context.AnimationTickCount = tickBase + static_cast<uint64_t>(tickIndex) + 1ULL;
        m_CurrentLayer->OnAnimationTick(m_Context);
    }
}

void Application::Update() {
    m_CurrentLayer->OnUpdate(m_Context);
}

void Application::Render() {
    m_Renderer.BeginFrame();

    if (m_CurrentLayer != nullptr) {
        m_CurrentLayer->OnRender(m_Context);
    }

    if (m_DebugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            overlay->OnRender(m_Context);
        }
    }

    m_Renderer.EndFrame();
}

void Application::DispatchEvent(const Event& event) {
    OnEvent(event);

    if (m_CurrentLayer != nullptr) {
        m_CurrentLayer->OnEvent(event);
    }

    if (m_DebugOverlayVisible) {
        for (const auto& overlay : m_Overlays) {
            overlay->OnEvent(event);
        }
    }
}

void Application::RefreshDisplayMetrics() {
    m_Context.Display = m_Window.GetDisplayMetrics();

    const bool scaleApplied = m_Renderer.ApplyDisplayScale(m_Context.Display.DisplayScale);
    if (!scaleApplied) {
        SOP_ASSERT(false, "SDL_SetRenderScale");
    }
}

void Application::OnEvent(const Event& event) {
    EventDispatcher::Dispatch<ApplicationStateChangeEvent>(
        event, [&](const ApplicationStateChangeEvent& stateEvent) mutable {
            ChangeState(stateEvent.NextState);
        });

    EventDispatcher::Dispatch<KeyEvent>(event, [&](const KeyEvent& keyEvent) {
        if (!keyEvent.Down)
            return;
        if (keyEvent.Key == SDLK_F1) {
            ToggleDebugOverlay();
            return;
        }
        if (keyEvent.Key == SDLK_ESCAPE) {
            if (m_Context.CurrentState == ApplicationState::Playing) {
                ChangeState(ApplicationState::Paused);
            } else if (m_Context.CurrentState == ApplicationState::Paused) {
                ChangeState(ApplicationState::Playing);
            }
        }
    });
}

void Application::OnApplicationStageChangeEvent() {
    const auto CreateLayerForState = [this](ApplicationState state) -> std::unique_ptr<Layer> {
        switch (state) {
            case ApplicationState::MainMenu:
            case ApplicationState::CharacterSelect:
            case ApplicationState::Paused:
            case ApplicationState::GameOver: {
                if (m_CurrentLayer != nullptr &&
                    dynamic_cast<UILayer*>(m_CurrentLayer.get()) != nullptr) {
                    return std::move(m_CurrentLayer);
                }
                return std::make_unique<UILayer>(m_Renderer, m_Window, m_EventDispatcher);
            }
            case ApplicationState::Playing: {
                // If we're already in a game layer, just reuse it and update the state. This
                // allows us to keep the game world loaded while pausing, showing game over
                // screen, etc.
                if (m_CurrentLayer != nullptr &&
                    dynamic_cast<GameLayer*>(m_CurrentLayer.get()) != nullptr) {
                    return std::move(m_CurrentLayer);
                }
                return std::make_unique<GameLayer>(m_Renderer, m_Window, m_EventDispatcher);
            }
            default:
                SOP_ASSERT(false, "Unhandled application state");
                return nullptr;
        }
    };
    spdlog::info("Current state: {}", static_cast<int32_t>(m_Context.CurrentState));

    m_CurrentLayer = CreateLayerForState(m_Context.CurrentState);
}

void Application::ChangeState(ApplicationState newState) {
    if (newState == ApplicationState::Playing && m_Context.Assets != nullptr) {
        (void)m_Context.Assets->getArenaBackgroundTexture(ArenaId::Chains);
        (void)m_Context.Assets->getArenaForegroundTexture(ArenaId::Chains);
        m_Context.Assets->preloadCharacterSpriteSheets(kDefaultCharacterId);
    }

    m_Context.CurrentState = newState;
    OnApplicationStageChangeEvent();
}

void Application::ToggleDebugOverlay() {
    m_DebugOverlayVisible = !m_DebugOverlayVisible;
    spdlog::info("Debug overlay {}", m_DebugOverlayVisible ? "enabled" : "disabled");
}
}  // namespace sop
