#include "smashorpass/app/Application.hpp"

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/layer/DebugLayer.hpp"
#include "smashorpass/layer/UILayer.hpp"
#include "smashorpass/layer/GameLayer.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace sop
{
    Application::Application() 
        : m_Window(WindowCreateInfo{ .Width = 1280, .Height = 720, .Title = "Smash Or Pass - The Game" }), 
        m_Renderer(m_Window)
    {
        m_Context.Assets = std::make_unique<AssetManager>(SOP_ASSET_ROOT_DIR, m_Renderer.NativeHandle());
        ChangeState(ApplicationState::MainMenu);
    }

    Application::~Application() 
    {
        /*while (!m_Overlays.empty()) {
            m_Overlays.pop_back();
        }*/
    }

    int Application::Run() 
    {
        bool running = true;
        spdlog::info("Starting the game");

        while(running) {
            ProcessEvents(running);
            Update();
            Render();
        }

        spdlog::info("Shutting down the game");
        return 0;
    }

    void Application::ProcessEvents(bool& running) 
    {
        // SDL events
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            {
                const auto& translatedEvent = TranslateSDLEvent(event);
                OnEvent(translatedEvent);
                m_CurrentLayer->OnEvent(translatedEvent);
            }

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Custom events
        while (!m_EventDispatcher.m_EventQueue.empty()) {
            Event customEvent = std::move(m_EventDispatcher.m_EventQueue.front());
            m_EventDispatcher.m_EventQueue.pop_front();
            OnEvent(customEvent);
            m_CurrentLayer->OnEvent(customEvent);
        }
    }

    void Application::Update() 
    {
        m_CurrentLayer->OnUpdate(m_Context);
    }

    void Application::Render() 
    {
        m_Renderer.BeginFrame();
        m_CurrentLayer->OnRender(m_Context);
        m_Renderer.EndFrame();
    }

    void Application::OnEvent(const Event& event) 
    {
        EventDispatcher::Dispatch<ApplicationStateChangeEvent>(event, [&](const ApplicationStateChangeEvent& stateEvent) mutable {
            ChangeState(stateEvent.NextState);
        });

        EventDispatcher::Dispatch<KeyEvent>(event, [&](const KeyEvent& keyEvent) {
            if (!keyEvent.Down)
                return;
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
                    if (m_CurrentLayer != nullptr && dynamic_cast<UILayer*>(m_CurrentLayer.get()) != nullptr) {
                        return std::move(m_CurrentLayer);
                    }
                    return std::make_unique<UILayer>(m_Renderer, m_Window, m_EventDispatcher);
                }
                case ApplicationState::Playing: {
                    // If we're already in a game layer, just reuse it and update the state. This
                    // allows us to keep the game world loaded while pausing, showing game over
                    // screen, etc.
                    if (m_CurrentLayer != nullptr && dynamic_cast<GameLayer*>(m_CurrentLayer.get()) != nullptr) {
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

    void Application::ChangeState(ApplicationState newState) 
    {
        m_Context.CurrentState = newState;
        OnApplicationStageChangeEvent();
    }
}
