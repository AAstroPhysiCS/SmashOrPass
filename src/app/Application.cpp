#include "smashorpass/app/Application.hpp"

#include "smashorpass/layer/DebugLayer.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace sop
{
    Application::Application() 
        : m_Window(WindowCreateInfo{ .Width = 1280, .Height = 720, .Title = "Smash Or Pass - The Game" }), 
        m_Renderer(m_Window)
    {
    }

    Application::~Application() {
        while (!m_Layers.empty()) {
            m_Layers.pop_back();
        }
    }

    int Application::Run() {
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

    void Application::ProcessEvents(bool& running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            const auto& translatedEvent = TranslateSDLEvent(event);
            for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
                (*it)->OnEvent(translatedEvent);
            }

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }

    void Application::Update() {
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
            (*it)->OnUpdate();
        }
    }

    void Application::Render() {
        m_Renderer.BeginFrame();
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
            (*it)->OnRender();
        }
        m_Renderer.EndFrame();
    }
}
