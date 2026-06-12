#include "smashorpass/state/states/in_game/Player.hpp"

#include <cmath>
#include <cstddef>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

namespace {

[[nodiscard]] SDL_FPoint SmallestPushOutOf(const SDL_FRect& player, const SDL_FRect& solid) {
    const float pushLeft = solid.x - (player.x + player.w);
    const float pushRight = solid.x + solid.w - player.x;
    const float pushUp = solid.y - (player.y + player.h);
    const float pushDown = solid.y + solid.h - player.y;

    const float horizontalPush = std::abs(pushLeft) < std::abs(pushRight) ? pushLeft : pushRight;
    const float verticalPush = std::abs(pushUp) < std::abs(pushDown) ? pushUp : pushDown;

    if (std::abs(verticalPush) <= std::abs(horizontalPush)) {
        return SDL_FPoint{.x = 0.0f, .y = verticalPush};
    }

    return SDL_FPoint{.x = horizontalPush, .y = 0.0f};
}

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

    const std::vector<SDL_FRect>& solidBoxes = arenaAsset.get().m_CollisionBoxes;
    const std::size_t maxPasses = solidBoxes.size() * 2;

    for (std::size_t pass = 0; pass < maxPasses; ++pass) {
        bool moved = false;

        for (const SDL_FRect& solidBox : solidBoxes) {
            if (!SDL_HasRectIntersectionFloat(&*playerCollisionBox, &solidBox)) {
                continue;
            }

            const SDL_FPoint push = SmallestPushOutOf(*playerCollisionBox, solidBox);

            m_Position.x += push.x;
            m_Position.y += push.y;
            playerCollisionBox->x += push.x;
            playerCollisionBox->y += push.y;

            resolution.Collided = true;
            if (push.y < 0.0f) {
                resolution.HitFloor = true;
            } else if (push.y > 0.0f) {
                resolution.HitCeiling = true;
            } else if (push.x > 0.0f) {
                resolution.HitLeftWall = true;
            } else if (push.x < 0.0f) {
                resolution.HitRightWall = true;
            }

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
    }

    if (resolution.HitCeiling && m_MovementState.Velocity.y < 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
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
    const MovementInput input = GatherMovementInput(ctx);

    TRY(grounded, QueryGroundInfo(ctx, arena));
    m_MovementState.Grounded = grounded;
    m_MovementState.FacingRight = m_FacingRight;

    const MovementResult movement =
        PlayerMovement::Tick(m_MovementState, input, m_MovementConfig);

    m_Position.x += movement.PositionDelta.x;
    m_Position.y += movement.PositionDelta.y;
    m_FacingRight = m_MovementState.FacingRight;

    TRY(collision, ResolveArenaCollisions(ctx, arena));
    ApplyCollisionResult(collision);

    m_State = movement.ActionState;
    return Ok();
}

}  // namespace sop
