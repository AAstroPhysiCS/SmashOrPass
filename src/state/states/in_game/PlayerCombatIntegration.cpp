#include <algorithm>
#include <cmath>

#include "smashorpass/state/states/in_game/CombatSystem.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"

namespace sop {

void Player::InitAttack() {
    m_PlayersHitByCurrentAttack.clear();
}

bool Player::HasHitPlayerThisAttack(const int playerId) const {
    return m_PlayersHitByCurrentAttack.contains(playerId);
}

void Player::MarkPlayerHitThisAttack(const int playerId) {
    m_PlayersHitByCurrentAttack.insert(playerId);
}

void Player::ReduceHealth(const float damage) {
    m_Health = std::max(0.0f, m_Health - damage);
}

void Player::LoseStock() {
    m_Stocks = std::max(0, m_Stocks - 1);
}

void Player::ResetStocks(const int stocks) {
    m_Stocks = std::max(0, stocks);
}

void Player::WinRound() {
    ++m_RoundsWon;
}

float Player::ApplyHit(const AttackData& attackData,
                       const HitResult& hitResult,
                       const bool attackerFacingRight) {
    if (!hitResult.hit) {
        return 0.0f;
    }

    const float previousHealth = m_Health;

    // hitResult; most vulnerable pixels were hit -> bestValue = 3, then 2, 1
    ReduceHealth(attackData.m_Damage * hitResult.bestValue);
    const float appliedDamage = previousHealth - m_Health;

    const float knockbackDirection = attackerFacingRight ? 1.0f : -1.0f;
    const float knockbackMultiplier = 1.0f + (100.0f - m_Health) / 100.0f;
    const float hitstunMultiplier = 1.0f + (100.0f - m_Health) / 200.0f;
    const int hitstunTicks = static_cast<int>(
        std::ceil(static_cast<float>(attackData.m_HitstunTicks) * hitstunMultiplier));

    m_MovementState.Velocity.x =
        attackData.m_Knockback.x * knockbackMultiplier * knockbackDirection;
    m_MovementState.Velocity.y = attackData.m_Knockback.y * knockbackMultiplier;
    m_MovementState.HitstunTicksRemaining = hitstunTicks;
    m_MovementState.Attack = MovementAttackState{};
    m_MovementState.Dash.TicksRemaining = 0;
    return appliedDamage;
}

}  // namespace sop
