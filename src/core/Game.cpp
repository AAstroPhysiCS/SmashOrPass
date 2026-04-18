#include "smashorpass/core/Game.hpp"
#include "smashorpass/core/Event.hpp"

#include <SDL3/SDL_keycode.h>

#include "smashorpass/rendering/Renderer.hpp"

#include "spdlog/spdlog.h"

namespace sop {

    void Game::OnEvent(const Event& event) {
        EventDispatcher::Dispatch<WindowResizeEvent>(event, [](const WindowResizeEvent& resizeEvent) {
            // Later: update viewport / UI layout / camera projection.
        });
    }

	void Game::Update(ApplicationState state) {
        switch (state) {
            case ApplicationState::MainMenu:
                //spdlog::info("In main menu");
                // TODO: main menu, handle menu input, etc.
                break;
            case ApplicationState::CharacterSelect:
                //spdlog::info("In character select");
                // TODO: character select, handle character select input, etc.
                break;
            case ApplicationState::Playing:
                //spdlog::info("Playing");
                // TODO: update game world, handle player input, etc.
                break;
            case ApplicationState::Paused:
                //spdlog::info("Paused");
                // TODO: pause menu, handle pause menu input, etc.
                break;
            case ApplicationState::GameOver:
                //spdlog::info("Game over");
                // TODO: game over screen, handle game over screen input, etc.
                break;
        }
    }

    void Game::Render(ApplicationState state, Renderer& renderer) {
        switch (state) {
            case ApplicationState::Playing: {
                RenderWorld(renderer);
                break;
            }
        }
    }
    
    void Game::RenderWorld(Renderer& renderer) {
        RenderStage(renderer);
        RenderPlayers(renderer);
        RenderEffects(renderer);
    }

    void Game::RenderStage(Renderer& renderer) {
        const SDL_Point size = renderer.GetCurrentOutputSize();

        // Sky/background
        renderer.FillRect(
            SDL_FRect{0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y)},
            Color{32, 36, 52, 255});

        // Main platform
        renderer.FillRect(SDL_FRect{static_cast<float>(size.x) * 0.25f,
                                    static_cast<float>(size.y) * 0.72f,
                                    static_cast<float>(size.x) * 0.50f,
                                    24.0f},
                          Color{110, 110, 120, 255});

        renderer.DrawRect(SDL_FRect{static_cast<float>(size.x) * 0.25f,
                                    static_cast<float>(size.y) * 0.72f,
                                    static_cast<float>(size.x) * 0.50f,
                                    24.0f},
                          Color{180, 180, 190, 255});

        // floating platform
        renderer.FillRect(SDL_FRect{static_cast<float>(size.x) * 0.42f,
                                    static_cast<float>(size.y) * 0.52f,
                                    static_cast<float>(size.x) * 0.16f,
                                    16.0f},
                          Color{90, 90, 100, 255});
    }

    void Game::RenderPlayers(Renderer& renderer) {
        // TODO: Placeholder fighters until sprite rendering is hooked up.
        renderer.FillRect(SDL_FRect{260.0f, 420.0f, 44.0f, 72.0f}, Color{220, 90, 90, 255});
        renderer.DrawRect(SDL_FRect{260.0f, 420.0f, 44.0f, 72.0f}, Color{255, 255, 255, 255});

        renderer.FillRect(SDL_FRect{520.0f, 420.0f, 44.0f, 72.0f}, Color{90, 140, 240, 255});
        renderer.DrawRect(SDL_FRect{520.0f, 420.0f, 44.0f, 72.0f}, Color{255, 255, 255, 255});
    }

    void Game::RenderEffects(Renderer& renderer) {
        // TODO:
        // attack trails
        // hit sparks
        // dust
        // particles
    }
}
