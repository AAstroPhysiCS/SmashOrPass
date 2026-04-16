#include "smashorpass/core/Game.hpp"

#include <SDL3/SDL_keycode.h>

#include "smashorpass/core/Event.hpp"

namespace sop {

    void Game::OnEvent(const Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<KeyEvent>([this](const KeyEvent& keyEvent) {
            if (!keyEvent.Down)
                return;

            if (keyEvent.Key == SDLK_ESCAPE) {
                if (m_State == GameState::Playing) {
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

	}

	void Game::ChangeState(GameState newState) {
		m_State = newState; 
	}
}
