#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/CombatSystem.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"
#include "smashorpass/state/states/in_game/MatchStats.hpp"

namespace sop {

namespace {

float ApplyHitResult(const HitResult& hitResult,
                     const WorldHitBox& attackerHitBox,
                     Player& attacker,
                     Player& defender) {
    if (!hitResult.hit) {
        return 0.0f;
    }

    const AttackData& attackData = attackerHitBox.hitBox.get().m_AttackData;

    const float appliedDamage = defender.ApplyHit(attackData, hitResult, attackerHitBox.facingRight);
    attacker.MarkPlayerHitThisAttack(defender.Id());
    return appliedDamage;
}

void RecordHitLocationStats(const HitResult& hitResult,
                            PlayerMatchStats& attackerStats,
                            PlayerMatchStats& defenderStats) {
    switch (hitResult.bestValue) {
        case 3:
            ++attackerStats.HeadHitsLanded;
            ++defenderStats.HeadHitsTaken;
            break;
        case 2:
            ++attackerStats.TorsoHitsLanded;
            ++defenderStats.TorsoHitsTaken;
            break;
        case 1:
            ++attackerStats.LegHitsLanded;
            ++defenderStats.LegHitsTaken;
            break;
        default:
            break;
    }
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
            const float appliedDamage =
                ApplyHitResult(hitResult, *attackerHitBox, attacker, defender);
            if (!hitResult.hit) {
                continue;
            }

            if (attackerIndex < m_MatchStats.size()) {
                ++m_MatchStats[attackerIndex].HitsLanded;
                m_MatchStats[attackerIndex].DamageDealt += appliedDamage;
            }

            if (defenderIndex < m_MatchStats.size()) {
                ++m_MatchStats[defenderIndex].HitsTaken;
                m_MatchStats[defenderIndex].DamageTaken += appliedDamage;
            }

            if (attackerIndex < m_MatchStats.size() && defenderIndex < m_MatchStats.size()) {
                RecordHitLocationStats(hitResult,
                                       m_MatchStats[attackerIndex],
                                       m_MatchStats[defenderIndex]);
            }
        }
    }

    return Ok();
}

}  // namespace sop
