#include "smashorpass/state/states/in_game/InGameState.hpp"

#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/CombatSystem.hpp"

namespace sop {

Result<void> InGameState::SolveCombat(AppCtx& ctx) {
    for (auto& debugData : m_PlayerCombatDebugData) {
        debugData = {};
    }

    for (std::size_t attackerIndex = 0; attackerIndex < m_Players.size(); ++attackerIndex) {
        Player& attacker = m_Players[attackerIndex];
        TRY(attackerHitBox, attacker.GetCurrentHitBox(ctx));
        if (!attackerHitBox) {
            continue;
        }

        for (std::size_t defenderIndex = 0; defenderIndex < m_Players.size(); ++defenderIndex) {
            if (attackerIndex == defenderIndex) {
                continue;
            }

            Player& defender = m_Players[defenderIndex];
            TRY(defenderHurtBox, defender.GetCurrentHurtBox(ctx));
            if (!defenderHurtBox) {
                continue;
            }

            const HitResult hitResult =
                detectOverlap(*attackerHitBox,
                              *defenderHurtBox,
                              &m_PlayerCombatDebugData[attackerIndex],
                              &m_PlayerCombatDebugData[defenderIndex]);
            (void)hitResult;
        }
    }

    return Ok();
}

}  // namespace sop
