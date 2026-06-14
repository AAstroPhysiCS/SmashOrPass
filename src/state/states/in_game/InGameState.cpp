#include "smashorpass/state/states/in_game/InGameState.hpp"

#include <chrono>
#include <cstddef>
#include <string>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/state/states/in_game/Defaults.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "smashorpass/util.hpp"

using Clock = std::chrono::steady_clock;

namespace sop {

constexpr int kGameLogicTicksPerSecond = 120; //120
constexpr int kGameLogicMaxCatchUpTicks = 10;
constexpr int kAnimationTicksPerSecond = 60; //60
constexpr int kAnimationMaxCatchUpTicks = 10;
// Derived from above
constexpr Clock::duration kGameLogicTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kGameLogicTicksPerSecond));
constexpr Clock::duration kAnimationTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kAnimationTicksPerSecond));

InGameState::InGameState(AppCtx& ctx,
                         Asset<ArenaAssetData> arenaAsset,
                         std::vector<Asset<CharacterAssetData>> characterAssets)
    : m_GameScreen(ctx),
      m_PauseScreen(ctx),
      m_ArenaAsset(std::move(arenaAsset)),
      m_CharacterAssets(std::move(characterAssets)) {
    UIBuilder gameScreenBuilder(m_GameScreen);
    m_GameScreen.Build(gameScreenBuilder);

    UIBuilder pauseScreenBuilder(m_PauseScreen);
    m_PauseScreen.Build(pauseScreenBuilder);

    ResetFrameTimer();
}

Result<void> InGameState::Initialize(AppCtx& ctx) {
    Arena defaultArena{.asset = m_ArenaAsset, .dimensions = SDL_Rect{}};
    defaultArena.ResizeToWindow(ctx.displayMetrics.LogicalSize());
    m_Arena = defaultArena;

    m_Players.clear();
    m_Players.reserve(m_CharacterAssets.size());

    for (std::size_t playerIndex = 0; playerIndex < m_CharacterAssets.size(); ++playerIndex) {
        InputTranslationHelper<InputAction> input;
        if (playerIndex < 2) {
            TRY_VOID(FillDefaultInputTranslation(input, static_cast<int>(playerIndex)));
        }

        const SDL_FPoint position = PlayerStartPosition(playerIndex);
        const bool facingRight = position.x < 960.0f;

        m_Players.emplace_back(static_cast<int>(playerIndex),
                               m_CharacterAssets[playerIndex],
                               position,
                               facingRight,
                               kDefaultPlayerHealth,
                               std::move(input));
    }

    m_PlayerDebugRenderOptions.clear();
    m_PlayerDebugRenderOptions.resize(m_Players.size());

    m_PlayerCombatDebugData.clear();
    m_PlayerCombatDebugData.resize(m_Players.size());

    m_CurrentRound = 1;
    m_Paused = false;
    ResetFrameTimer();
    return Ok();
}

Result<EventFlow> InGameState::OnEvent(AppCtx& ctx, const Event& event) {
    // Resume match from pause menu
    if (const auto* navigation = std::get_if<NavigationEvent>(&event.Payload)) {
        if (navigation->Action == NavigationAction::ResumeMatch) {
            m_Paused = false;
            ResetFrameTimer();
            return Ok(EventFlow::Consumed);
        }
        return Ok(EventFlow::Passed);
    }

    // Go into pause menu
    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat && keyEvent->Key == SDLK_ESCAPE) {
            TogglePause();
            return Ok(EventFlow::Consumed);
        }
    }

    // Handle pause menu event handling
    if (m_Paused) {
        const EventFlow pauseUiFlow = m_PauseScreen.OnEvent(ctx, event);
        if (pauseUiFlow == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }
        return Ok(EventFlow::Passed);
    }

    // Handle game screen event handling
    const EventFlow gameUiFlow = m_GameScreen.OnEvent(ctx, event);
    if (gameUiFlow == EventFlow::Consumed) {
        return Ok(EventFlow::Consumed);
    }

    // Handle player specific event handling
    // TODO: Think about whether player event handling should always
    // fall through or not (see the EventFlow::Passed below)
    for (Player& player : m_Players) {
        TRY_VOID(player.OnEvent(ctx, event));
    }

    EventDispatcher::Dispatch<PlayerParticleEffectEvent>(
        event, [&](const PlayerParticleEffectEvent& particleEffectEvent) {
            switch (particleEffectEvent.Type) {
                case PlayerParticleEffectType::SwordFire: {
                    sop::util::EmitSwordFireParticleEffect(ctx.particleSystem, particleEffectEvent);
                    break;
                }
                case PlayerParticleEffectType::DashBlue: {
                    sop::util::EmitDashParticleEffect(ctx.particleSystem, particleEffectEvent);
                    break;
                }
            }
        });

    return Ok(EventFlow::Passed);
}

Result<void> InGameState::OnUpdate(AppCtx& ctx) {
    const Clock::time_point now = Clock::now();
    const Clock::duration elapsed = now - m_PreviousUpdateTime;
    m_PreviousUpdateTime = now;
    const std::chrono::duration<float> dt = std::chrono::duration<float>(elapsed);

    if (m_Paused) {
        m_PauseScreen.OnUpdate(ctx);
        return Ok();
    }

    // Game Logic
    int gameLogicTicks = 0;
    while (now - m_PreviousGameLogicTick >= kGameLogicTickDuration &&
           gameLogicTicks < kGameLogicMaxCatchUpTicks) {
        TRY_VOID(TickGameLogic(ctx));
        m_PreviousGameLogicTick += kGameLogicTickDuration;
        ++gameLogicTicks;
    }
    if (gameLogicTicks == kGameLogicMaxCatchUpTicks) {
        m_PreviousGameLogicTick = now;
    }

    // ---- Update Ticks
    // Animations
    int animationTicks = 0;
    while (now - m_PreviousAnimationTick >= kAnimationTickDuration &&
           animationTicks < kAnimationMaxCatchUpTicks) {
        TRY_VOID(TickAnimation(ctx));
        m_PreviousAnimationTick += kAnimationTickDuration;
        ++animationTicks;
    }
    if (animationTicks == kAnimationMaxCatchUpTicks) {
        m_PreviousAnimationTick = now;
    }

    // Effects (every frame)
    TRY_VOID(TickEffects(ctx, dt));

    TRY_VOID(SolveCombat(ctx));
    TRY_VOID(ResolveDeathsAndRespawns());

    SyncGameScreen();
    m_GameScreen.OnUpdate(ctx);
    return Ok();
}

void InGameState::SyncGameScreen() {
    if (m_Players.size() < 2) {
        return;
    }

    m_GameScreen.SetPlayersStats(m_Players[0].Health(),
                                 m_Players[0].Stocks(),
                                 m_Players[0].RoundsWon(),
                                 m_Players[1].Health(),
                                 m_Players[1].Stocks(),
                                 m_Players[1].RoundsWon(),
                                 m_CurrentRound);
}

Result<void> InGameState::OnRender(AppCtx& ctx) {
    TRY_VOID(AdjustToWindow(ctx));
    TRY_VOID(RenderBackdrop(ctx));
    TRY_VOID(RenderPlayers(ctx));
    TRY_VOID(RenderEffects(ctx));
    TRY_VOID(RenderForeground(ctx));
    TRY_VOID(RenderPlayerMarkers(ctx));
    TRY_VOID(RenderArenaCollisionBoxes(ctx));
    TRY_VOID(RenderDebugBoxes(ctx));
    TRY_VOID(RenderUi(ctx));
    return Ok();
}

void InGameState::ResetFrameTimer() {
    const Clock::time_point now = Clock::now();
    m_PreviousUpdateTime = now;
    m_PreviousGameLogicTick = now;
    m_PreviousAnimationTick = now;
}

void InGameState::TogglePause() {
    m_Paused = !m_Paused;
    ResetFrameTimer();
}

Result<void> InGameState::AdjustToWindow(AppCtx& ctx) {
    m_Arena.ResizeToWindow(ctx.displayMetrics.LogicalSize());
    return Ok();
}

Result<void> InGameState::TickGameLogic(AppCtx& ctx) {
    for (Player& player : m_Players) {
        TRY_VOID(player.TickGameLogic(ctx, m_Arena));
    }

    TRY_VOID(SolveCollisions(ctx));

    return Ok();
}

Result<void> InGameState::SolveCollisions(AppCtx& ctx) {
    for (Player& player : m_Players) {
        TRY_VOID(player.SyncCollisionBodyToPosition(ctx));
        player.ResetCollisionForTick();
    }

    for (Player& player : m_Players) {
        TRY_VOID(player.ResolveArenaCollisionsForTick(ctx, m_Arena));
    }
    // for >2 players you may want to run this 2-3 times
    for (std::size_t first = 0; first < m_Players.size(); ++first) {
        for (std::size_t second = first + 1; second < m_Players.size(); ++second) {
            TRY_VOID(m_Players[first].ResolveCollisionWithPlayerForTick(m_Players[second]));
        }
    }

    for (Player& player : m_Players) {
        player.ApplyCollisionBodyToPosition();
        player.ApplyCollisionResult();
    }

    return Ok();
}

Result<void> InGameState::TickAnimation(AppCtx& ctx) {
    for (Player& player : m_Players) {
        TRY_VOID(player.TickAnimations(ctx, m_Arena));
    }
    return Ok();
}

Result<void> InGameState::TickEffects(AppCtx& ctx, std::chrono::duration<float> dt) {
    ctx.particleSystem.Update(dt.count());
    return Ok();
}

}  // namespace sop
