#pragma once

#include <SDL3/SDL_rect.h>

#include <optional>
#include <random>
#include <vector>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/state/states/in_game/Arena.hpp"
#include "smashorpass/state/states/in_game/PlayerMovement.hpp"
#include "smashorpass/util.hpp"

namespace sop {

enum class InputAction { MOVE_LEFT, MOVE_RIGHT, JUMP, DASH, ATTACK };

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
    
    struct CollisionResolution {
        bool Collided = false;
        bool HitFloor = false;
        bool HitCeiling = false;
        bool HitLeftWall = false;
        bool HitRightWall = false;
    };
    
    Result<bool> QueryGroundInfo(AppCtx& ctx, const Arena& arena) const;
    [[nodiscard]] MovementInput GatherMovementInput(AppCtx& ctx);

    [[nodiscard]] Result<CollisionResolution> ResolveArenaCollisions(AppCtx& ctx,
                                                                     const Arena& arena);
    void ApplyCollisionResult(const CollisionResolution& resolution);

    int m_playerId;
    Asset<CharacterAssetData> m_Asset;

    InputTranslationHelper<InputAction> m_InputTranslationHelper;
    std::vector<InputAction> m_InputQueue;

    /// Player animation anchor in arena baseline coordinates.
    SDL_FPoint m_Position;
    bool m_FacingRight;
    PlayerActionState m_State;
    MovementConfig m_MovementConfig;
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

    // ---- Stats
    float m_Health;

    [[nodiscard]] Result<CharacterAnimation> GetAnimationToShow(AppCtx& ctx,
                                                                const Arena& arena) const;
    [[nodiscard]] std::optional<SDL_FRect> GetBaselineSpriteRect(
        const CharacterSpriteSheetFrame& frame) const;
    [[nodiscard]] Result<std::optional<SDL_FRect>> GetBaselineCollisionBox(AppCtx& ctx) const;
};

}  // namespace sop
