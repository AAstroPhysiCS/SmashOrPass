#pragma once

#include "Event.hpp"
#include "GameState.hpp"

namespace sop {

    struct GameConfig {

    };

    inline static GameConfig loadDefault() {
        return GameConfig{}; 
    }

    class Game final {
    public:
        void OnEvent(const Event& event);
        void Update();

        inline GameState State() const { return m_State; }
    private:
        void ChangeState(GameState newState);

        GameState m_State = GameState::MainMenu;
    };
}
