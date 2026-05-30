#pragma once

#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"

namespace sop {

struct CombatDebugData {
    std::optional<SDL_FRect> attackerSpriteRect;
    std::unordered_map<int, SDL_FRect> attackerHitBoxBounds;
    std::optional<SDL_FRect> defenderSpriteRect;
    std::vector<SDL_FRect> defenderSubHurtBounds;
};

struct HitResult {
    bool hit = false;
    int bestValue = 0;
    int overlapCount = 0;
    int outerOverlapCount = 0;
    int innerOverlapCount = 0;
};

HitResult detectOverlap(
    const WorldHitBox& attackerHitbox,
    const WorldHurtBox& defenderHurtbox,
    CombatDebugData* debug = nullptr
);

}