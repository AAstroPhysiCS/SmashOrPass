#pragma once

#include <cstdint>

namespace sop {

enum class ApplicationState : uint8_t { MainMenu, Playing, CharacterSelect, Paused, GameOver };
}