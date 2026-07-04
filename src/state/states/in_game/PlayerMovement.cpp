#include "smashorpass/state/states/in_game/PlayerMovement.hpp"

#include <algorithm>

namespace sop {

namespace {

void TickCooldowns(MovementState& state, const MovementInput& input) {
    if (state.Dash.CooldownTicksRemaining > 0) {
        --state.Dash.CooldownTicksRemaining;
    }

    if (!state.Attack.IsActive()) {
        return;
    }

    --state.Attack.TicksRemaining;

    if (state.Attack.MinimumTicksRemaining > 0) {
        --state.Attack.MinimumTicksRemaining;
    }

    if (state.Attack.MinimumTicksRemaining == 0 && !input.AttackHeld) {
        state.Attack.TicksRemaining = 0;
    }
}

void RefreshAirOptions(MovementState& state) {
    state.Dash.AirDashAvailable = true;
    state.DashJumpAvailable = false;
}

void RefreshJumpWindows(MovementState& state,
                        const MovementInput& input,
                        const MovementConfig& config) {
    if (state.Grounded) {
        state.GroundJumpGraceTicksRemaining = config.GroundJumpGraceTicks;
    }

    if (input.JumpPressed) {
        state.JumpBufferTicksRemaining = std::max(1, config.JumpBufferTicks);
    }
}

void TickJumpWindows(MovementState& state) {
    if (!state.Grounded && state.GroundJumpGraceTicksRemaining > 0) {
        --state.GroundJumpGraceTicksRemaining;
    }

    if (state.JumpBufferTicksRemaining > 0) {
        --state.JumpBufferTicksRemaining;
    }
}

void TryStartDash(MovementState& state, const MovementInput& input, const MovementConfig& config) {
    if (!input.DashPressed) {
        return;
    }

    if (state.Dash.IsActive() || state.Dash.CooldownTicksRemaining != 0) {
        return;
    }

    if (state.Attack.MinimumTicksRemaining > 0) {
        return;
    }

    if (!state.Grounded && !state.Dash.AirDashAvailable) {
        return;
    }

    const float horizontalIntent = input.HorizontalIntent();
    if (horizontalIntent != 0.0f) {
        state.Dash.Direction = horizontalIntent;
    } else {
        state.Dash.Direction = state.FacingRight ? 1.0f : -1.0f;
    }

    state.FacingRight = state.Dash.Direction > 0.0f;
    state.Dash.TicksRemaining = config.DashTicks;
    state.Dash.CooldownTicksRemaining = config.DashCooldownTicks;
    state.Attack = MovementAttackState{};

    if (!state.Grounded) {
        state.Dash.AirDashAvailable = false;
        state.DashJumpAvailable = true;
    }
}

void ApplyActiveDash(MovementState& state, const MovementConfig& config) {
    state.Velocity.x = state.Dash.Direction * config.DashSpeed;
    state.Velocity.y = 0.0f;
}

void TryStartAttack(MovementState& state,
                    const MovementInput& input,
                    const MovementConfig& config) {
    if (state.Attack.IsActive() || !input.AttackPressed) {
        return;
    }

    state.Attack.TicksRemaining = config.TotalAttackTicks;
    state.Attack.MinimumTicksRemaining = config.MinAttackTicks;
    state.Velocity.x *= config.AttackVelocityMultiplier;
    state.Velocity.y *= config.AttackVelocityMultiplier;
}

void TryApplyJump(MovementState& state, const MovementConfig& config) {
    if (state.JumpBufferTicksRemaining <= 0) {
        return;
    }

    if (state.Grounded || state.GroundJumpGraceTicksRemaining > 0) {
        state.Velocity.y = config.JumpVelocity;
        state.GroundJumpGraceTicksRemaining = 0;
        state.JumpBufferTicksRemaining = 0;
    } else if (state.DashJumpAvailable) {
        state.Velocity.y = config.JumpVelocity;
        state.DashJumpAvailable = false;
        state.JumpBufferTicksRemaining = 0;
    }
}

void TryApplyMove(MovementState& state, const MovementInput& input, const MovementConfig& config) {
    const float horizontalIntent = input.HorizontalIntent();
    if (horizontalIntent == 0.0f) {
        return;
    }

    const float movementFactor = state.Grounded ? 1.0f : config.AirMovementFactor;
    state.Velocity.x += horizontalIntent * config.WalkAcceleration * movementFactor;
    state.FacingRight = horizontalIntent > 0.0f;
}

void ApplyGravity(MovementState& state,
                  const MovementConfig& config,
                  const float multiplier = 1.0f) {
    state.Velocity.y += config.Gravity * multiplier;
}

void ClampFallSpeed(MovementState& state, const MovementConfig& config) {
    state.Velocity.y = std::min(state.Velocity.y, config.MaxFallSpeed);
}

void ApplyHorizontalDrag(MovementState& state,
                         const MovementConfig& config,
                         const float multiplier = 1.0f) {
    const float friction =
        (state.Grounded ? config.GroundFriction : config.AirFriction) * multiplier;
    if (state.Velocity.x > 0.0f) {
        state.Velocity.x = std::max(state.Velocity.x - friction, 0.0f);
    } else if (state.Velocity.x < 0.0f) {
        state.Velocity.x = std::min(state.Velocity.x + friction, 0.0f);
    }
}

void ClampWalkSpeed(MovementState& state, const MovementConfig& config) {
    state.Velocity.x = std::clamp(state.Velocity.x, -config.WalkSpeed, config.WalkSpeed);
}

void ApplyPassivePhysics(MovementState& state,
                         const MovementConfig& config,
                         const float gravityMultiplier = 1.0f,
                         const float frictionMultiplier = 1.0f) {
    ApplyGravity(state, config, gravityMultiplier);
    ClampFallSpeed(state, config);
    ApplyHorizontalDrag(state, config, frictionMultiplier);
    ClampWalkSpeed(state, config);
}

PlayerActionState ApplyMoves(MovementState& state,
                             const MovementInput& input,
                             const MovementConfig& config) {
    if (state.HitstunTicksRemaining > 0) {
        --state.HitstunTicksRemaining;
        ApplyGravity(state, config);
        ClampFallSpeed(state, config);
        return PlayerActionState::HITSTUN;
    }

    TryStartDash(state, input, config);
    if (state.Dash.IsActive()) {
        ApplyActiveDash(state, config);
        --state.Dash.TicksRemaining;
        return PlayerActionState::DASHING;
    }

    TryStartAttack(state, input, config);
    if (state.Attack.IsActive()) {
        ApplyPassivePhysics(
            state, config, config.AttackGravityMultiplier, config.AttackFrictionMultiplier);
        return PlayerActionState::ATTACKING;
    }

    TryApplyJump(state, config);
    TryApplyMove(state, input, config);
    ApplyPassivePhysics(state, config);

    if (input.HorizontalIntent() != 0.0f) {
        return PlayerActionState::RUNNING;
    }

    return PlayerActionState::IDLE;
}

}  // namespace

MovementResult PlayerMovement::Tick(MovementState& state,
                                    const MovementInput& input,
                                    const MovementConfig& config) {
    TickCooldowns(state, input);
    RefreshJumpWindows(state, input, config);

    if (state.Grounded) {
        RefreshAirOptions(state);
    }

    const PlayerActionState actionState = ApplyMoves(state, input, config);
    TickJumpWindows(state);

    return MovementResult{
        .PositionDelta = state.Velocity,
        .ActionState = actionState,
    };
}

}  // namespace sop
