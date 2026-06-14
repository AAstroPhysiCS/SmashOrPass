#include "smashorpass/state/states/in_game/Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/CollisionSystem.hpp"

namespace sop {

namespace {

[[nodiscard]] bool IsActionHeld(const Input& input,
                                const InputTranslationHelper<InputAction>& translation,
                                InputAction action) {
    const auto* keys = translation.GetKeysForAction(action);
    if (keys == nullptr) {
        return false;
    }

    for (SDL_Keycode key : *keys) {
        if (input.GetKeyPressInfo(key)) {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool HasQueuedAction(const std::vector<InputAction>& inputQueue, InputAction action) {
    for (const InputAction queuedAction : inputQueue) {
        if (queuedAction == action) {
            return true;
        }
    }

    return false;
}

}  // namespace

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
        m_MovementState.Velocity.x =
            std::clamp(actualVelocityX, m_MovementState.Velocity.x, 0.0f);
    }

    if (collision.hitPlayerRight && m_MovementState.Velocity.x > 0.0f) {
        m_MovementState.Velocity.x =
            std::clamp(actualVelocityX, 0.0f, m_MovementState.Velocity.x);
    }
}

MovementInput Player::GatherMovementInput(AppCtx& ctx) {
    MovementInput input{};
    input.JumpPressed = HasQueuedAction(m_InputQueue, InputAction::JUMP);
    input.DashPressed = HasQueuedAction(m_InputQueue, InputAction::DASH);
    input.AttackPressed = HasQueuedAction(m_InputQueue, InputAction::ATTACK);
    input.MoveLeftHeld = IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::MOVE_LEFT);
    input.MoveRightHeld =
        IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::MOVE_RIGHT);
    input.AttackHeld = IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::ATTACK);
    m_InputQueue.clear();
    return input;
}

Result<void> Player::TickGameLogic(AppCtx& ctx, const Arena& arena) {
    (void)arena;

    const MovementInput input = GatherMovementInput(ctx);

    m_MovementState.FacingRight = m_FacingRight;

    const MovementResult movement =
        PlayerMovement::Tick(m_MovementState, input, m_MovementConfig);

    m_Position.x += movement.PositionDelta.x;
    m_Position.y += movement.PositionDelta.y;
    m_FacingRight = m_MovementState.FacingRight;

    m_MovementState.Grounded = false;

    m_State = movement.ActionState;
    return Ok();
}

}  // namespace sop
