#pragma once

#include <cstddef>
#include <vector>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/ArenaAsset.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/state/states/in_game/GameMode.hpp"
#include "smashorpass/state/states/in_game/MatchStats.hpp"
#include "smashorpass/state/states/in_game/PlayerControl.hpp"

namespace sop {

struct MatchConfig {
    GameMode Mode = GameMode::Smash;
    int TargetRoundsToWin = 3;
    int StocksPerRound = 3;
    Asset<ArenaAssetData> ArenaAsset{};
    std::vector<Asset<CharacterAssetData>> CharacterAssets{};
    std::vector<PlayerControl> PlayerControls{};
};

struct MatchResults {
    std::size_t WinnerIndex = 0;
    int Player1RoundsWon = 0;
    int Player2RoundsWon = 0;
    PlayerMatchStats Player1Stats{};
    PlayerMatchStats Player2Stats{};
};

}  // namespace sop
