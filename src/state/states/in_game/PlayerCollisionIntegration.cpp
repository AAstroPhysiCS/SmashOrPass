#include <algorithm>
#include <cmath>
#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/CollisionSystem.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"

namespace sop {

Result<void> Player::SyncCollisionBodyToPosition(AppCtx& ctx) {
    TRY_VOID(EnsureCollisionProfile(ctx));
    if (!m_CollisionProfileInitialized) {
        return Ok();
    }

    SyncCollisionBodyToAnchor();
    m_CollisionBodyAfterMovement = m_CollisionBody.Rect;
    return Ok();
}

void Player::ResetCollisionForTick() {
    ResetCollisionContacts(m_CollisionBody);
}

Result<void> Player::ResolveArenaCollisionsForTick(AppCtx& ctx, const Arena& arena) {
    if (!m_CollisionProfileInitialized) {
        return Ok();
    }

    TRY(arenaAsset, ctx.assets.GetAssetData(arena.asset));

    const std::vector<SDL_FRect>& solidBoxes = arenaAsset.get().m_CollisionBoxes;
    const std::size_t maxPasses = solidBoxes.size() * 2;

    for (std::size_t pass = 0; pass < maxPasses; ++pass) {
        bool moved = false;

        for (const SDL_FRect& solidBox : solidBoxes) {
            const bool collisionResolved =
                ResolveCollision(m_CollisionBody, solidBox, m_MovementState.Velocity.y);
            moved = moved || collisionResolved;
        }

        if (!moved) {
            break;
        }
    }

    return Ok();
}

Result<void> Player::ResolveCollisionWithPlayerForTick(Player& other) {
    if (!m_CollisionProfileInitialized || !other.m_CollisionProfileInitialized) {
        return Ok();
    }

    (void)ResolveCollision(m_CollisionBody,
                           other.m_CollisionBody,
                           m_MovementState.Velocity.y,
                           other.m_MovementState.Velocity.y);
    return Ok();
}

void Player::ApplyCollisionBodyToPosition() {
    if (!m_CollisionProfileInitialized) {
        return;
    }

    constexpr float kCollisionEpsilon = 0.00001f;
    const float dx = m_CollisionBody.Rect.x - m_CollisionBodyAfterMovement.x;
    const float dy = m_CollisionBody.Rect.y - m_CollisionBodyAfterMovement.y;
    if (std::abs(dx) <= kCollisionEpsilon && std::abs(dy) <= kCollisionEpsilon) {
        return;
    }

    m_Position.x += dx;
    m_Position.y += dy;
}

void Player::ApplyCollisionResult() {
    const CollisionContacts& collision = m_CollisionBody.Contacts;

    const float previousBodyX = m_CollisionBodyAfterMovement.x - m_MovementState.Velocity.x;
    const float actualVelocityX = m_CollisionBody.Rect.x - previousBodyX;

    if (collision.hitGround && m_MovementState.Velocity.y > 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
        m_MovementState.Grounded = true;
        m_MovementState.Dash.AirDashAvailable = true;
        m_MovementState.DashJumpAvailable = false;
    }

    if (collision.hitCeiling && m_MovementState.Velocity.y < 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
    }

    if (collision.hitWallLeft && m_MovementState.Velocity.x < 0.0f) {
        m_MovementState.Velocity.x = 0.0f;
    }

    if (collision.hitWallRight && m_MovementState.Velocity.x > 0.0f) {
        m_MovementState.Velocity.x = 0.0f;
    }

    if (collision.hitPlayerLeft && m_MovementState.Velocity.x < 0.0f) {
        m_MovementState.Velocity.x = std::clamp(actualVelocityX, m_MovementState.Velocity.x, 0.0f);
    }

    if (collision.hitPlayerRight && m_MovementState.Velocity.x > 0.0f) {
        m_MovementState.Velocity.x = std::clamp(actualVelocityX, 0.0f, m_MovementState.Velocity.x);
    }
}

}  // namespace sop
