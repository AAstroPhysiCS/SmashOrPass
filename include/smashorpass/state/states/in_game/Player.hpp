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

   private:
    [[nodiscard]] static Vec2 LocalFramePointToBaselinePoint(const CharacterSpriteSheetFrame& frame,
                                                             const SDL_FRect& spriteRect,
                                                             Vec2 localPoint,
                                                             bool facingRight);

    [[nodiscard]] static Vec2 MapBaselinePointToArenaPoint(Vec2 point,
                                                           const SDL_Rect& arenaDimensions);

    // ---- Mechanical
    int m_playerId;
    Asset<CharacterAssetData> m_Asset;

    InputTranslationHelper<InputAction> m_InputTranslationHelper;
    std::vector<InputAction> m_InputQueue;

    /// Player animation anchor in arena baseline coordinates.
    SDL_FPoint m_Position;
    bool m_FacingRight;
    PlayerState m_State;

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
