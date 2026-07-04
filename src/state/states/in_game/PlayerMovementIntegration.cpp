#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"

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
    return TickGameLogic(ctx, arena, GatherMovementInput(ctx));
}

Result<void> Player::TickGameLogic(AppCtx& ctx, const Arena& arena, const MovementInput& input) {
    (void)arena;
    (void)ctx;

    m_MovementState.FacingRight = m_FacingRight;
    const bool wasAttacking = m_MovementState.Attack.IsActive();

    const MovementResult movement = PlayerMovement::Tick(m_MovementState, input, m_MovementConfig);
    if (!wasAttacking && m_MovementState.Attack.IsActive()) {
        InitAttack();
    }

    m_Position.x += movement.PositionDelta.x;
    m_Position.y += movement.PositionDelta.y;
    m_FacingRight = m_MovementState.FacingRight;

    // Grounded will be set by collision detection
    m_MovementState.Grounded = false;

    m_State = movement.ActionState;
    return Ok();
}

void Player::Respawn(const SDL_FPoint position, const bool facingRight, const float health) {
    m_Position = position;
    m_FacingRight = facingRight;
    m_State = PlayerActionState::IDLE;
    m_MovementState = MovementState{};
    m_MovementState.FacingRight = facingRight;
    m_Health = health;
    m_InputQueue.clear();
    m_PlayersHitByCurrentAttack.clear();
    m_CurrentAnimation = CharacterAnimation::Idle;
    m_CurrentAnimationFrame = 0;
    SyncCollisionBodyToAnchor();
}

}  // namespace sop
