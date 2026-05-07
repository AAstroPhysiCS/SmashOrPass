#pragma once

#include <cstdint>

namespace sop {

enum class ApplicationState : uint8_t {
    None = 0,
    MainMenu = 1 << 0,
    Playing = 1 << 1,
    CharacterSelect = 1 << 2,
    Paused = 1 << 3,
    GameOver = 1 << 4
};

}  // namespace sop