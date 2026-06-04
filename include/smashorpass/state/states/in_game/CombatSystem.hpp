#pragma once

#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"
#include "smashorpass/state/states/in_game/DebugData.hpp"

namespace sop {

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
    PlayerCombatDebugData* attackerDebugData = nullptr,
    PlayerCombatDebugData* defenderDebugData = nullptr
);

}