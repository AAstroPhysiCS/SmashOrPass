#include "smashorpass/app/Application.hpp"

#include "smashorpass/core/Game.hpp"
#include "smashorpass/debug/DebugOverlay.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace sop
{
    Application::Application() 
        : m_Window(WindowCreateInfo{ .Width = 1280, .Height = 720, .Title = "Smash Or Pass - The Game" }), 
        m_Renderer(m_Window),
        m_DebugOverlay(m_Window, m_Renderer) {
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
            m_DebugOverlay.ProcessEvent(event);

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }

    void Application::Update() {
        m_Game.Update();
    }

    void Application::Render() {
        m_Renderer.BeginFrame();
        
        m_DebugOverlay.BeginFrame();
        m_DebugOverlay.Draw();
        m_DebugOverlay.EndFrame();

        m_Renderer.EndFrame();
    }
}
