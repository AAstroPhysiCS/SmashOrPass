#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/CombatSystem.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"

namespace sop {

namespace {

void ApplyHitResult(const HitResult& hitResult,
                    const WorldHitBox& attackerHitBox,
                    Player& attacker,
                    Player& defender) {
    if (!hitResult.hit) {
        return;
    }

    const AttackData& attackData = attackerHitBox.hitBox.get().m_AttackData;

    defender.ApplyHit(attackData, hitResult, attackerHitBox.facingRight);
    attacker.MarkPlayerHitThisAttack(defender.Id());
}

}  // namespace

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
            if (attacker.HasHitPlayerThisAttack(defender.Id())) {
                continue;
            }
            TRY(defenderHurtBox, defender.GetCurrentHurtBox(ctx));
            if (!defenderHurtBox) {
                continue;
            }

            const HitResult hitResult = detectOverlap(*attackerHitBox,
                                                      *defenderHurtBox,
                                                      &m_PlayerCombatDebugData[attackerIndex],
                                                      &m_PlayerCombatDebugData[defenderIndex]);
            ApplyHitResult(hitResult, *attackerHitBox, attacker, defender);
        }
    }

    return Ok();
}

}  // namespace sop
