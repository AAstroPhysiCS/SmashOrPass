#pragma once

#include <SDL3/SDL_rect.h>

#include <chrono>
#include <vector>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/state/states/in_game/Arena.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"
#include "smashorpass/state/states/in_game/ui/GameScreen.hpp"
#include "smashorpass/state/states/in_game/ui/PauseScreen.hpp"

namespace sop {

class InGameState final : public State {
   public:
    explicit InGameState(AppCtx& ctx,
                         ArenaAssetHandle arenaAsset = {},
                         std::vector<CharacterAssetHandle> characterAssets = {});
    ~InGameState() override = default;

    sop_util::Result<void> Initialize(AppCtx& ctx) final;

    [[nodiscard]] std::string_view DebugName() const final {
        return "InGame";
    }

    sop_util::Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    sop_util::Result<void> OnUpdate(AppCtx& ctx) final;
    sop_util::Result<void> OnRender(AppCtx& ctx) final;

   private:
    // ---- Internal Functions
    void ResetFrameTimer();
    void TogglePause();

    sop_util::Result<void> AdjustToWindow(AppCtx& ctx);

    sop_util::Result<void> TickGameLogic(AppCtx& ctx);
    sop_util::Result<void> TickAnimation(AppCtx& ctx);
    sop_util::Result<void> TickEffects(AppCtx& ctx, std::chrono::duration<float> dt);

    sop_util::Result<void> RenderBackdrop(AppCtx& ctx);
    sop_util::Result<void> RenderPlayers(AppCtx& ctx);
    sop_util::Result<void> RenderEffects(AppCtx& ctx);
    sop_util::Result<void> RenderForeground(AppCtx& ctx);
    sop_util::Result<void> RenderCollisionBoxes(AppCtx& ctx);
    sop_util::Result<void> RenderUi(AppCtx& ctx);

    // ---- Internal Variables
    GameScreen m_GameScreen;
    PauseScreen m_PauseScreen;

    Arena m_Arena;
    ArenaAssetHandle m_ArenaAsset;
    std::vector<CharacterAssetHandle> m_CharacterAssets;
    // Player 1 is at index 0, Player 2 at 1, ...
    std::vector<Player> m_Players;

    using Clock = std::chrono::steady_clock;
    Clock::time_point m_PreviousUpdateTime;
    Clock::time_point m_PreviousGameLogicTick;
    Clock::time_point m_PreviousAnimationTick;
    bool m_Paused = false;
};

}  // namespace sop
