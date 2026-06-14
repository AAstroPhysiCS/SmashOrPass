#include "smashorpass/state/states/in_game/Player.hpp"

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

void Player::ApplyHit(const AttackData& attackData,
                      const HitResult& hitResult,
                      const bool attackerFacingRight) {
    if (!hitResult.hit) {
        return;
    }

    m_Health -= attackData.m_Damage;

    const float knockbackDirection = attackerFacingRight ? 1.0f : -1.0f;
    m_MovementState.Velocity.x = attackData.m_Knockback.x * knockbackDirection;
    m_MovementState.Velocity.y = attackData.m_Knockback.y;
    m_MovementState.HitstunTicksRemaining = attackData.m_HitstunTicks;
    m_MovementState.Attack = MovementAttackState{};
    m_MovementState.Dash.TicksRemaining = 0;
}

}  // namespace sop
