#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

enum class PlayerActionState {
    IDLE,
    RUNNING,
    ATTACKING,
    DASHING,
    HITSTUN,
};

struct MovementInput {
    bool JumpPressed = false;
    bool DashPressed = false;
    bool AttackPressed = false;
    bool MoveLeftHeld = false;
    bool MoveRightHeld = false;
    bool AttackHeld = false;

    [[nodiscard]] float HorizontalIntent() const {
        if (MoveLeftHeld == MoveRightHeld) {
            return 0.0f;
        }

        return MoveLeftHeld ? -1.0f : 1.0f;
    }
};

struct MovementConfig {
    float WalkSpeed = 5.0f;
    float WalkAcceleration = 1.0f;
    float Gravity = 0.28f;
    float MaxFallSpeed = 20.0f;
    float JumpVelocity = -10.0f;
    float AirMovementFactor = 0.2f;
    float GroundFriction = 0.1f;
    float AirFriction = 0.04f;

    int DashTicks = 20;
    int DashCooldownTicks = 54;
    float DashSpeed = 11.0f;

    int TotalAttackTicks = 72;
    int MinAttackTicks = 28;
};

struct MovementDashState {
    int TicksRemaining = 0;
    int CooldownTicksRemaining = 0;
    float Direction = 1.0f;
    bool AirDashAvailable = true;

    [[nodiscard]] bool IsActive() const {
        return TicksRemaining > 0;
    }
};

struct MovementAttackState {
    int TicksRemaining = 0;
    int MinimumTicksRemaining = 0;

    [[nodiscard]] bool IsActive() const {
        return TicksRemaining > 0;
    }
};

struct MovementState {
    SDL_FPoint Velocity{0.0f, 0.0f};
    bool Grounded = false;
    bool FacingRight = true;
    bool DashJumpAvailable = false;
    int HitstunTicksRemaining = 0;

    MovementDashState Dash{};
    MovementAttackState Attack{};
};

struct MovementResult {
    SDL_FPoint PositionDelta{0.0f, 0.0f};
    bool FacingRightChanged = false;
    PlayerActionState ActionState = PlayerActionState::IDLE;
};

class PlayerMovement {
   public:
    [[nodiscard]] static MovementResult Tick(MovementState& state,
                                             const MovementInput& input,
                                             const MovementConfig& config);
};

}  // namespace sop
