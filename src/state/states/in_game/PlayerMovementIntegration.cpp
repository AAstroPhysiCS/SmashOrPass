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

Result<Player::CollisionResolution> Player::ResolveArenaCollisions(AppCtx& ctx,
                                                                   const Arena& arena) {
    CollisionResolution resolution{};

    TRY(arenaAsset, ctx.assets.GetAssetData(arena.asset));
    TRY(playerCollisionBox, GetBaselineCollisionBox(ctx));
    if (!playerCollisionBox) {
        return Ok(resolution);
    }

    CollisionBody playerBody{
        .Rect = *playerCollisionBox,
        .Push = PushState{.canPushLeft = true,
                          .canPushRight = true,
                          .canPushUp = true,
                          .canPushDown = false},
        .Dynamic = true,
        .Weight = 100.0f,
    };

    const std::vector<SDL_FRect>& solidBoxes = arenaAsset.get().m_CollisionBoxes;
    const std::size_t maxPasses = solidBoxes.size() * 2;

    for (std::size_t pass = 0; pass < maxPasses; ++pass) {
        bool moved = false;

        for (const SDL_FRect& solidBox : solidBoxes) {
            const CollisionSolveResult collision =
                pushBoxes(playerBody, solidBox, m_MovementState.Velocity.y);
            if (!collision.Collided) {
                continue;
            }

            m_Position.x += collision.Push.x;
            m_Position.y += collision.Push.y;
            playerCollisionBox->x = playerBody.Rect.x;
            playerCollisionBox->y = playerBody.Rect.y;

            resolution.Collided = true;
            resolution.HitFloor = resolution.HitFloor || collision.HitFloor;
            resolution.HitCeiling = resolution.HitCeiling || collision.HitCeiling;
            resolution.HitWallOnLeft = resolution.HitWallOnLeft || collision.HitWallOnLeft;
            resolution.HitWallOnRight = resolution.HitWallOnRight || collision.HitWallOnRight;
            moved = true;
        }

        if (!moved) {
            break;
        }
    }

    return Ok(resolution);
}

Result<bool> Player::QueryGroundInfo(AppCtx& ctx, const Arena& arena) const {
    TRY(onGround, IsOnGround(ctx, arena));
    return Ok(onGround);
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

void Player::ApplyCollisionBodyRect(const SDL_FRect& previousRect, const SDL_FRect& resolvedRect) {
    constexpr float kCollisionEpsilon = 0.00001f;

    const float dx = resolvedRect.x - previousRect.x;
    const float dy = resolvedRect.y - previousRect.y;
    if (std::abs(dx) <= kCollisionEpsilon && std::abs(dy) <= kCollisionEpsilon) {
        return;
    }

    m_Position.x += dx;
    m_Position.y += dy;

    CollisionResolution resolution{.Collided = true};
    if (dy < -kCollisionEpsilon) {
        resolution.HitFloor = true;
    } else if (dy > kCollisionEpsilon) {
        resolution.HitCeiling = true;
    } else if (dx > kCollisionEpsilon) {
        resolution.HitWallOnLeft = true;
    } else if (dx < -kCollisionEpsilon) {
        resolution.HitWallOnRight = true;
    }
    ApplyCollisionResult(resolution);
}

Result<void> Player::ResolveCollisionWithPlayer(AppCtx& ctx, Player& other) {
    TRY(firstCollisionBox, GetBaselineCollisionBox(ctx));
    TRY(secondCollisionBox, other.GetBaselineCollisionBox(ctx));
    if (!firstCollisionBox || !secondCollisionBox) {
        return Ok();
    }

    CollisionBody firstBody{
        .Rect = *firstCollisionBox,
        .Push = PushState{.canPushLeft = true,
                          .canPushRight = true,
                          .canPushUp = true,
                          .canPushDown = false},
        .Dynamic = true,
        .Weight = 100.0f,
    };
    CollisionBody secondBody{
        .Rect = *secondCollisionBox,
        .Push = PushState{.canPushLeft = true,
                          .canPushRight = true,
                          .canPushUp = true,
                          .canPushDown = false},
        .Dynamic = true,
        .Weight = 100.0f,
    };

    const CollisionSolveResult collision = pushBoxes(firstBody,
                                                     secondBody,
                                                     m_MovementState.Velocity.y,
                                                     other.m_MovementState.Velocity.y);
    if (!collision.Collided) {
        return Ok();
    }

    ApplyCollisionBodyRect(*firstCollisionBox, firstBody.Rect);
    other.ApplyCollisionBodyRect(*secondCollisionBox, secondBody.Rect);
    return Ok();
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

    TRY(collision, ResolveArenaCollisions(ctx, arena));
    ApplyCollisionResult(collision);

    m_State = movement.ActionState;
    return Ok();
}

}  // namespace sop
