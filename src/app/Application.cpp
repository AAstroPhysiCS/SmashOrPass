#include "smashorpass/app/Application.hpp"

#include "smashorpass/core/Game.hpp"
#include "smashorpass/debug/DebugOverlay.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

#include <SDL.h>
#include <spdlog/spdlog.h>

namespace sop
{
    Application::Application() 
        : m_Window(WindowCreateInfo{ .Width = 1280, .Height = 720, .Title = "Smash Or Pass - The Game" }), 
        m_Renderer(m_Window) {
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
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    void Application::Update() {
        m_Game.Update();
    }

    void Application::Render() {
        m_Renderer.BeginFrame();
        //m_DebugOverlay.Draw();
        m_Renderer.EndFrame();
    }
}
