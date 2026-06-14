#include "smashorpass/state/states/in_game/Player.hpp"

#include <algorithm>

#include "smashorpass/state/states/in_game/CombatSystem.hpp"

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

void Player::ApplyHit(const AttackData& attackData,
                      const HitResult& hitResult,
                      const bool attackerFacingRight) {
    if (!hitResult.hit) {
        return;
    }

    ReduceHealth(attackData.m_Damage);

    const float knockbackDirection = attackerFacingRight ? 1.0f : -1.0f;
    const float knockbackMultiplier = 1.0f + (100.0f - m_Health) / 100.0f;
    m_MovementState.Velocity.x =
        attackData.m_Knockback.x * knockbackMultiplier * knockbackDirection;
    m_MovementState.Velocity.y = attackData.m_Knockback.y * knockbackMultiplier;
    m_MovementState.HitstunTicksRemaining = attackData.m_HitstunTicks;
    m_MovementState.Attack = MovementAttackState{};
    m_MovementState.Dash.TicksRemaining = 0;
}

}  // namespace sop
