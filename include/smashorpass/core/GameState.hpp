#pragma once

#include <cstdint>

namespace sop {

    enum class GameState : uint8_t { 
        MainMenu, 
        Playing, 
        CharacterSelect,
        Paused, 
        GameOver 
    };
}