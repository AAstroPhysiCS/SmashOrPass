#pragma once

#include <SDL3/SDL_rect.h>

#include <optional>
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
   private:
    // ---- Mechanical
    int m_playerId;
    CharacterAssetHandle m_Asset;
    InputTranslationHelper<InputAction> m_InputTranslationHelper;
    std::vector<InputAction> m_InputQueue;
    /// Player animation anchor in arena baseline coordinates.
    SDL_FPoint m_Position;
    bool m_FacingRight;
    PlayerState m_State;
    CharacterAnimation m_CurrentAnimation = CharacterAnimation::Idle;
    int m_CurrentAnimationFrame = 0;
    float m_VelocityY = 0.0f;
    int m_DashTicksRemaining = 0;
    int m_DashCooldownTicksRemaining = 0;
    float m_DashDirection = 1.0f;
    bool m_AirDashAvailable = true;
    bool m_DashJumpAvailable = false;

    // ---- Stats
    float m_Health;

    [[nodiscard]] CharacterAnimation GetAnimationToShow(AppCtx& ctx, const Arena& arena) const;
    [[nodiscard]] std::optional<SDL_FRect> GetBaselineSpriteRect(
        const CharacterSpriteSheetFrame& frame) const;
    [[nodiscard]] std::optional<SDL_FRect> GetBaselineCollisionBox(AppCtx& ctx) const;

   public:
    Player(int playerId,
           CharacterAssetHandle asset,
           SDL_FPoint position,
           bool facingRight,
           float health,
           InputTranslationHelper<InputAction> inputTranslationHelper);

    sop_util::Result<void> OnEvent(AppCtx& ctx, const Event& event);
    void TickGameLogic(AppCtx& ctx, const Arena& arena);
    void TickAnimations(AppCtx& ctx, const Arena& arena);
    [[nodiscard]] bool IsOnGround(AppCtx& ctx, const Arena& arena) const;
    sop_util::Result<void> Render(AppCtx& ctx, const Arena& arena) const;
    sop_util::Result<void> RenderCollisionBox(AppCtx& ctx, const Arena& arena) const;
};

}  // namespace sop
