#pragma once

#include <SDL3/SDL_rect.h>

#include <chrono>
#include <vector>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/state/states/in_game/Arena.hpp"
#include "smashorpass/state/states/in_game/DebugData.hpp"
#include "smashorpass/state/states/in_game/GameMode.hpp"
#include "smashorpass/state/states/in_game/Player.hpp"
#include "smashorpass/state/states/in_game/ui/GameScreen.hpp"
#include "smashorpass/state/states/in_game/ui/PauseScreen.hpp"

namespace sop {

struct PlayerDebugRenderOptions {
    bool enabled = true;
    bool collisionBox = false;
    bool hitBoxes = false;
    bool hurtBoxes = false;
    bool combatHitBoxes = false;
    bool combatHurtBoxes = false;
};

class InGameState final : public State {
   public:
    explicit InGameState(AppCtx& ctx,
                         Asset<ArenaAssetData> arenaAsset = {},
                         std::vector<Asset<CharacterAssetData>> characterAssets = {},
                         GameMode gameMode = GameMode::Smash);
    ~InGameState() override = default;

    Result<void> Initialize(AppCtx& ctx) final;

    [[nodiscard]] std::string_view DebugName() const final {
        return "InGame";
    }

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

    // to keep Debug Menu flexible and allow for >2 Players
    [[nodiscard]] std::size_t GetPlayerCount() const {
        return m_Players.size();
    }

    [[nodiscard]] PlayerDebugRenderOptions& GetPlayerDebugOptions(std::size_t index) {
        return m_PlayerDebugRenderOptions.at(index);
    }

    [[nodiscard]] const PlayerDebugRenderOptions& GetPlayerDebugOptions(std::size_t index) const {
        return m_PlayerDebugRenderOptions.at(index);
    }

   private:
    // ---- Internal Functions
    void ResetFrameTimer();
    void TogglePause();

    Result<void> AdjustToWindow(AppCtx& ctx);

    Result<void> TickGameLogic(AppCtx& ctx);
    Result<void> SolveCollisions(AppCtx& ctx);
    Result<void> TickAnimation(AppCtx& ctx);
    Result<void> TickEffects(AppCtx& ctx, std::chrono::duration<float> dt);
    Result<void> SolveCombat(AppCtx& ctx);
    void SyncGameScreen();
    Result<void> ResolveDeathsAndRespawns();
    void StartNextRound(std::size_t winnerIndex);
    void RestartRound();

    Result<void> RenderBackdrop(AppCtx& ctx);
    Result<void> RenderPlayers(AppCtx& ctx);
    Result<void> RenderPlayerMarkers(AppCtx& ctx);
    Result<void> RenderEffects(AppCtx& ctx);
    Result<void> RenderForeground(AppCtx& ctx);
    Result<void> RenderArenaCollisionBoxes(AppCtx& ctx);
    Result<void> RenderDebugBoxes(AppCtx& ctx);
    Result<void> RenderUi(AppCtx& ctx);

    // ---- Internal Variables
    GameScreen m_GameScreen;
    PauseScreen m_PauseScreen;

    Arena m_Arena;
    Asset<ArenaAssetData> m_ArenaAsset;
    std::vector<Asset<CharacterAssetData>> m_CharacterAssets;
    GameMode m_GameMode = GameMode::Smash;
    // Player 1 is at index 0, Player 2 at 1, ...
    std::vector<Player> m_Players;
    std::vector<PlayerDebugRenderOptions> m_PlayerDebugRenderOptions;
    std::vector<PlayerCombatDebugData> m_PlayerCombatDebugData;

    using Clock = std::chrono::steady_clock;
    Clock::time_point m_PreviousUpdateTime;
    Clock::time_point m_PreviousGameLogicTick;
    Clock::time_point m_PreviousAnimationTick;
    int m_CurrentRound = 1;
    bool m_Paused = false;
};

}  // namespace sop
