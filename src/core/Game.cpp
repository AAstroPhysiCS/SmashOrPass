#include "smashorpass/core/Game.hpp"

#include <SDL3/SDL_keycode.h>

#include "smashorpass/core/Event.hpp"

#include "spdlog/spdlog.h"

namespace sop {

    void Game::OnEvent(const Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<KeyEvent>([this](const KeyEvent& keyEvent) {
            if (!keyEvent.Down)
                return;

            if (keyEvent.Key == SDLK_ESCAPE) {
                //TODO: Delete MainMenu path, testing only rn
                if (m_State == GameState::Playing || m_State == GameState::MainMenu) {
                    ChangeState(GameState::Paused);
                } else if (m_State == GameState::Paused) {
                    ChangeState(GameState::Playing);
                }
            }

            if (keyEvent.Key == SDLK_RETURN && m_State == GameState::MainMenu) {
                ChangeState(GameState::Playing);
            }
        });

        dispatcher.Dispatch<GameStateChangeEvent>([this](const GameStateChangeEvent& stateEvent) {
            ChangeState(stateEvent.NextState);
        });

        dispatcher.Dispatch<WindowResizeEvent>([](const WindowResizeEvent& resizeEvent) {
            // Later: update viewport / UI layout / camera projection.
        });
    }

	void Game::Update()	{
        switch (m_State) {
            case GameState::MainMenu:
                //spdlog::info("In main menu");
                // TODO: main menu, handle menu input, etc.
                break;
            case GameState::CharacterSelect:
                //spdlog::info("In character select");
                // TODO: character select, handle character select input, etc.
                break;
            case GameState::Playing:
                //spdlog::info("Playing");
                // TODO: update game world, handle player input, etc.
                break;
            case GameState::Paused:
                //spdlog::info("Paused");
                // TODO: pause menu, handle pause menu input, etc.
                break;
            case GameState::GameOver:
                //spdlog::info("Game over");
                // TODO: game over screen, handle game over screen input, etc.
                break;
        }
	}

	void Game::ChangeState(GameState newState) {
		m_State = newState; 
	}
}
