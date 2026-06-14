#include "smashorpass/state/states/in_game/Player.hpp"

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
    m_CollisionBodyBeforeSolve = m_CollisionBody.Rect;
    return Ok();
}

void Player::ResetCollisionForTick() {
    ResetPushState(m_CollisionBody);
    m_CollisionResolutionThisTick = {};
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
            const CollisionSolveResult collision =
                pushBoxes(m_CollisionBody, solidBox, m_MovementState.Velocity.y);
            if (!collision.Collided) {
                continue;
            }

            m_CollisionResolutionThisTick.Collided = true;
            m_CollisionResolutionThisTick.HitFloor =
                m_CollisionResolutionThisTick.HitFloor || collision.HitFloor;
            m_CollisionResolutionThisTick.HitCeiling =
                m_CollisionResolutionThisTick.HitCeiling || collision.HitCeiling;
            m_CollisionResolutionThisTick.HitWallOnLeft =
                m_CollisionResolutionThisTick.HitWallOnLeft || collision.HitWallOnLeft;
            m_CollisionResolutionThisTick.HitWallOnRight =
                m_CollisionResolutionThisTick.HitWallOnRight || collision.HitWallOnRight;
            moved = true;
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

    const SDL_FRect firstBodyBefore = m_CollisionBody.Rect;
    const SDL_FRect secondBodyBefore = other.m_CollisionBody.Rect;

    const CollisionSolveResult collision = pushBoxes(m_CollisionBody,
                                                     other.m_CollisionBody,
                                                     m_MovementState.Velocity.y,
                                                     other.m_MovementState.Velocity.y);
    if (!collision.Collided) {
        return Ok();
    }

    const auto accumulateDelta = [](CollisionResolution& resolution,
                                    const SDL_FRect& previousRect,
                                    const SDL_FRect& resolvedRect) {
        constexpr float kCollisionEpsilon = 0.00001f;

        const float dx = resolvedRect.x - previousRect.x;
        const float dy = resolvedRect.y - previousRect.y;
        if (std::abs(dx) <= kCollisionEpsilon && std::abs(dy) <= kCollisionEpsilon) {
            return;
        }

        resolution.Collided = true;
        if (dy < -kCollisionEpsilon) {
            resolution.HitFloor = true;
        } else if (dy > kCollisionEpsilon) {
            resolution.HitCeiling = true;
        } else if (dx > kCollisionEpsilon) {
            resolution.HitWallOnLeft = true;
        } else if (dx < -kCollisionEpsilon) {
            resolution.HitWallOnRight = true;
        }
    };

    accumulateDelta(m_CollisionResolutionThisTick, firstBodyBefore, m_CollisionBody.Rect);
    accumulateDelta(other.m_CollisionResolutionThisTick,
                    secondBodyBefore,
                    other.m_CollisionBody.Rect);
    return Ok();
}

void Player::ApplyCollisionBodyToPosition() {
    if (!m_CollisionProfileInitialized) {
        return;
    }

    constexpr float kCollisionEpsilon = 0.00001f;
    const float dx = m_CollisionBody.Rect.x - m_CollisionBodyBeforeSolve.x;
    const float dy = m_CollisionBody.Rect.y - m_CollisionBodyBeforeSolve.y;
    if (std::abs(dx) <= kCollisionEpsilon && std::abs(dy) <= kCollisionEpsilon) {
        return;
    }

    m_Position.x += dx;
    m_Position.y += dy;
}

void Player::ApplyCollisionResult() {
    ApplyCollisionResult(m_CollisionResolutionThisTick);
}

void Player::ApplyCollisionResult(const CollisionResolution& resolution) {
    if (resolution.HitFloor && m_MovementState.Velocity.y > 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
        m_MovementState.Grounded = true;
        m_MovementState.Dash.AirDashAvailable = true;
        m_MovementState.DashJumpAvailable = false;
    }

    if (resolution.HitCeiling && m_MovementState.Velocity.y < 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
    }

    if (resolution.HitWallOnLeft && m_MovementState.Velocity.x < 0.0f) {
        m_MovementState.Velocity.x = 0.0f;
    }

    if (resolution.HitWallOnRight && m_MovementState.Velocity.x > 0.0f) {
        m_MovementState.Velocity.x = 0.0f;
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

    //TRY(grounded, QueryGroundInfo(ctx, arena));
    //m_MovementState.Grounded = grounded;
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
