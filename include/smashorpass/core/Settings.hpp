#pragma once

#include <SDL3/SDL_keycode.h>

#include <algorithm>
#include <array>

namespace sop {

struct PlayerKeyBindings {
    SDL_Keycode MoveLeft = SDLK_UNKNOWN;
    SDL_Keycode MoveRight = SDLK_UNKNOWN;
    SDL_Keycode Jump = SDLK_UNKNOWN;
    SDL_Keycode Dash = SDLK_UNKNOWN;
    SDL_Keycode Attack = SDLK_UNKNOWN;
};

struct Settings {
    static constexpr int kMinRounds = 1;
    static constexpr int kMaxRounds = 9;
    static constexpr int kMinStocks = 1;
    static constexpr int kMaxStocks = 9;
    static constexpr int kMinDeathmatchOutOfBoundsDamage = 0;
    static constexpr int kMaxDeathmatchOutOfBoundsDamage = 100;
    static constexpr int kPlayerKeyBindingCount = 2;

    int DeathmatchRoundsToWin = 3;
    int SmashRoundsToWin = 3;
    int SmashStocksPerRound = 3;
    int DeathmatchOutOfBoundsDamage = 0;

    std::array<PlayerKeyBindings, kPlayerKeyBindingCount> PlayerKeyBindingsByPlayer{
        PlayerKeyBindings{.MoveLeft = SDLK_A,
                          .MoveRight = SDLK_D,
                          .Jump = SDLK_W,
                          .Dash = SDLK_LSHIFT,
                          .Attack = SDLK_SPACE},
        PlayerKeyBindings{.MoveLeft = SDLK_LEFT,
                          .MoveRight = SDLK_RIGHT,
                          .Jump = SDLK_UP,
                          .Dash = SDLK_RSHIFT,
                          .Attack = SDLK_RCTRL},
    };

    void Clamp() {
        DeathmatchRoundsToWin = std::clamp(DeathmatchRoundsToWin, kMinRounds, kMaxRounds);
        SmashRoundsToWin = std::clamp(SmashRoundsToWin, kMinRounds, kMaxRounds);
        SmashStocksPerRound = std::clamp(SmashStocksPerRound, kMinStocks, kMaxStocks);
        DeathmatchOutOfBoundsDamage = std::clamp(DeathmatchOutOfBoundsDamage,
                                                 kMinDeathmatchOutOfBoundsDamage,
                                                 kMaxDeathmatchOutOfBoundsDamage);
    }
};

}  // namespace sop
