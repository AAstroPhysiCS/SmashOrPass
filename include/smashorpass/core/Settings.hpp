#pragma once

#include <algorithm>

namespace sop {

struct Settings {
    static constexpr int kMinRounds = 1;
    static constexpr int kMaxRounds = 9;
    static constexpr int kMinStocks = 1;
    static constexpr int kMaxStocks = 9;

    int DeathmatchRoundsToWin = 3;
    int SmashRoundsToWin = 3;
    int SmashStocksPerRound = 3;

    void Clamp() {
        DeathmatchRoundsToWin = std::clamp(DeathmatchRoundsToWin, kMinRounds, kMaxRounds);
        SmashRoundsToWin = std::clamp(SmashRoundsToWin, kMinRounds, kMaxRounds);
        SmashStocksPerRound = std::clamp(SmashStocksPerRound, kMinStocks, kMaxStocks);
    }
};

}  // namespace sop
