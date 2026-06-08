#pragma once

#include <SDL3/SDL_rect.h>

#include <optional>
#include <random>
#include <vector>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/state/states/in_game/Arena.hpp"
#include "smashorpass/util.hpp"

namespace sop {

enum class InputAction { MOVE_LEFT, MOVE_RIGHT, JUMP, DASH, ATTACK };

enum class PlayerState {
    IDLE,
    RUNNING,
    ATTACKING,
    DASHING,
};

struct WorldHitBox {
    std::reference_wrapper<const HitBox> hitBox;
    SDL_FRect spriteRect;
    bool facingRight;
};

struct WorldHurtBox {
    std::reference_wrapper<const HurtBox> hurtBox;
    SDL_FRect spriteRect;
    bool facingRight;
};

class Player {
   public:
    Player(int playerId,
           Asset<CharacterAssetData> asset,
           SDL_FPoint position,
           bool facingRight,
           float health,
           InputTranslationHelper<InputAction> inputTranslationHelper);
    Result<void> OnEvent(AppCtx& ctx, const Event& event);

    Result<void> TickGameLogic(AppCtx& ctx, const Arena& arena);
    Result<void> Movement(AppCtx& ctx, const Arena& arena);
    Result<void> TickAnimations(AppCtx& ctx, const Arena& arena);

    [[nodiscard]] Result<bool> IsOnGround(AppCtx& ctx, const Arena& arena) const;

    Result<void> Render(AppCtx& ctx, const Arena& arena) const;
    Result<void> RenderCollisionBox(AppCtx& ctx, const Arena& arena) const;
    Result<void> RenderHitBoxes(AppCtx& ctx, const Arena& arena) const;
    Result<void> RenderHurtBoxes(AppCtx& ctx, const Arena& arena) const;
    
    [[nodiscard]] Result<std::optional<WorldHitBox>> GetCurrentHitBox(
    AppCtx& ctx) const;
    [[nodiscard]] Result<std::optional<WorldHurtBox>> GetCurrentHurtBox(
        AppCtx& ctx) const;
    
    
   private:
    [[nodiscard]] static Vec2 LocalFramePointToBaselinePoint(const CharacterSpriteSheetFrame& frame,
                                                         const SDL_FRect& spriteRect,
                                                         Vec2 localPoint,
                                                         bool facingRight);

    [[nodiscard]] static Vec2 MapBaselinePointToArenaPoint(Vec2 point,
                                                       const SDL_Rect& arenaDimensions);
    
    struct CurrentPlayerInput {
        bool JumpPressed = false;
        bool DashPressed = false;
        bool AttackPressed = false;
        bool MoveLeftHeld = false;
        bool MoveRightHeld = false;
        bool AttackHeld = false;
    };

    struct GroundInfo {
        bool OnGround = false;
    };

    struct HorizontalStepResult {
        bool DashedThisTick = false;
        bool MovedHorizontally = false;
    };

    struct CollisionResolution {
        bool Collided = false;
        bool HitFloor = false;
        bool HitCeiling = false;
        bool HitLeftWall = false;
        bool HitRightWall = false;
    };

    struct AttackState {
        int TicksRemaining = 0;
        int MinimumTicksRemaining = 0;

        [[nodiscard]] bool IsActive() const {
            return TicksRemaining > 0;
        }
    };

    struct MovementState {
        SDL_FPoint Position{};
        bool isGrounded = false;
        bool FacingRight = true;
        PlayerState State = PlayerState::IDLE;

        SDL_FPoint Velocity{0.f, 0.f};

        int DashTicksRemaining = 0;
        int DashCooldownTicksRemaining = 0;
        float DashDirection = 1.0f;
        bool DashJumpAvailable = false;
        bool AirDashAvailable = true;
        AttackState AttackInfo{};
    };
    
    
    [[nodiscard]] CurrentPlayerInput GatherInput(AppCtx& ctx);
    void TickCooldowns(const CurrentPlayerInput& input);

    [[nodiscard]] Result<GroundInfo> QueryGroundInfo(AppCtx& ctx, const Arena& arena) const;
    void RefreshAirOptionsFromGround(const GroundInfo& groundInfo);
    void RefreshPostMoveAirOptions(const GroundInfo& groundInfo,
                                   const HorizontalStepResult& horizontal);

    void TryStartDash(const CurrentPlayerInput& input);
    void TryStartAttack(const CurrentPlayerInput& input);
    [[nodiscard]] HorizontalStepResult ApplyHorizontalIntent(const CurrentPlayerInput& input);
    void TryApplyJump(const CurrentPlayerInput& input);
    void ApplyVerticalMotion(const HorizontalStepResult& horizontal);

    [[nodiscard]] Result<CollisionResolution> ResolveArenaCollisions(AppCtx& ctx,
                                                                     const Arena& arena);
    void ApplyCollisionResult(const CollisionResolution& resolution);

    void UpdatePlayerState(const CurrentPlayerInput& input);


    void Movement_Dash();
    [[nodiscard]] bool IsDashActive() const {
        return m_MovementState.DashTicksRemaining > 0;
    }
    void RefreshAirOptions();
    void ApplyMoves(const CurrentPlayerInput& input);
    void TryApplyMove(const CurrentPlayerInput& input);
    void ApplyGravity();
    void ApplyFriction();
    void ApplyVelocity();

    int m_playerId;
    Asset<CharacterAssetData> m_Asset;

    InputTranslationHelper<InputAction> m_InputTranslationHelper;
    std::vector<InputAction> m_InputQueue;

    /// Player animation anchor in arena baseline coordinates.
    SDL_FPoint m_Position;
    bool m_FacingRight;
    PlayerState m_State;
    MovementState m_MovementState;

    // ---- Effect tracking for animation frames
    Result<void> DispatchSwordFrameEffects(AppCtx& ctx,
                                           const Arena& arena,
                                           const CharacterSpriteSheetFrame& frame,
                                           std::span<const FrameEffectMask> swordMasks);
    std::mt19937 m_EffectRandom{std::random_device{}()};

    CharacterAnimation m_CurrentAnimation = CharacterAnimation::Idle;
    int m_CurrentAnimationFrame = 0;

    CharacterAnimation m_PreviousEffectAnimation = CharacterAnimation::Idle;
    int m_PreviousEffectFrameIndex = -1;

    float m_VelocityY = 0.0f;

    int m_DashTicksRemaining = 0;
    int m_DashCooldownTicksRemaining = 0;
    float m_DashDirection = 1.0f;
    bool m_DashJumpAvailable = false;

    bool m_AirDashAvailable = true;

    // ---- Stats
    float m_Health;

    [[nodiscard]] Result<CharacterAnimation> GetAnimationToShow(AppCtx& ctx,
                                                                const Arena& arena) const;
    [[nodiscard]] std::optional<SDL_FRect> GetBaselineSpriteRect(
        const CharacterSpriteSheetFrame& frame) const;
    [[nodiscard]] Result<std::optional<SDL_FRect>> GetBaselineCollisionBox(AppCtx& ctx) const;
};

}  // namespace sop
