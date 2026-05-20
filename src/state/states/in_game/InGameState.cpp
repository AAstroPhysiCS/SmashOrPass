#include "smashorpass/state/states/in_game/InGameState.hpp"

#include <chrono>
#include <cstddef>
#include <string>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/core/Color.hpp"
#include "smashorpass/state/states/in_game/Defaults.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "smashorpass/util.hpp"

using Clock = std::chrono::steady_clock;

namespace sop {

constexpr int kGameLogicTicksPerSecond = 120;
constexpr int kGameLogicMaxCatchUpTicks = 10;
constexpr int kAnimationTicksPerSecond = 60;
constexpr int kAnimationMaxCatchUpTicks = 10;
// Derived from above
constexpr Clock::duration kGameLogicTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kGameLogicTicksPerSecond));
constexpr Clock::duration kAnimationTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kAnimationTicksPerSecond));

InGameState::InGameState(AppCtx& ctx,
                         ArenaAssetHandle arenaAsset,
                         std::vector<CharacterAssetHandle> characterAssets)
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
    constexpr float kDefaultPlayerHealth = 100.0f;

    Arena defaultArena{.asset = m_ArenaAsset, .dimensions = SDL_Rect{}};
    defaultArena.ResizeToWindow(ctx.DisplayMetrics.LogicalSize());
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

    // Effects (every frame)
    TRY_VOID(TickEffects(ctx, dt));

    m_GameScreen.OnUpdate(ctx);
    return Ok();
}

Result<void> InGameState::OnRender(AppCtx& ctx) {
    TRY_VOID(AdjustToWindow(ctx));
    TRY_VOID(RenderBackdrop(ctx));
    TRY_VOID(RenderPlayers(ctx));
    TRY_VOID(RenderEffects(ctx));
    TRY_VOID(RenderForeground(ctx));
    TRY_VOID(RenderCollisionBoxes(ctx));
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
    m_Arena.ResizeToWindow(ctx.DisplayMetrics.LogicalSize());
    return Ok();
}

Result<void> InGameState::TickGameLogic(AppCtx& ctx) {
    for (Player& player : m_Players) {
        player.TickGameLogic(ctx, m_Arena);
    }
    return Ok();
}

Result<void> InGameState::TickAnimation(AppCtx& ctx) {
    for (Player& player : m_Players) {
        player.TickAnimations(ctx, m_Arena);
    }
    return Ok();
}

Result<void> InGameState::TickEffects(AppCtx& ctx, std::chrono::duration<float> dt) {
    ctx.ParticleSystem.Update(dt.count());
    return Ok();
}

Result<void> InGameState::RenderBackdrop(AppCtx& ctx) {
    TRY(arenaAsset, ctx.Assets.Get(m_Arena.asset));

    SDL_FRect rect{};
    SDL_RectToFRect(&m_Arena.dimensions, &rect);
    return ctx.Renderer.DrawTexture(arenaAsset.get().m_Background.get(), rect);
}

Result<void> InGameState::RenderPlayers(AppCtx& ctx) {
    for (const Player& player : m_Players) {
        TRY_VOID(player.Render(ctx, m_Arena));
    }
    return Ok();
}

Result<void> InGameState::RenderEffects(AppCtx&) {
    return Ok();
}

Result<void> InGameState::RenderForeground(AppCtx& ctx) {
    TRY(arenaAsset, ctx.Assets.Get(m_Arena.asset));

    SDL_FRect rect{};
    SDL_RectToFRect(&m_Arena.dimensions, &rect);
    return ctx.Renderer.DrawTexture(arenaAsset.get().m_Foreground.get(), rect);
}

Result<void> InGameState::RenderCollisionBoxes(AppCtx& ctx) {
    if (!ctx.RenderCollisionBoxes || !m_Arena.asset) {
        return Ok();
    }

    TRY(arenaAsset, ctx.Assets.Get(m_Arena.asset));

    for (const SDL_FRect& collisionBox : arenaAsset.get().m_CollisionBoxes) {
        TRY_VOID(ctx.Renderer.DrawRect(MapBaselineRectToArena(collisionBox, m_Arena.dimensions),
                                         Color{0, 255, 0, 255}));
    }

    for (const Player& player : m_Players) {
        TRY_VOID(player.RenderCollisionBox(ctx, m_Arena));
    }

    return Ok();
}

Result<void> InGameState::RenderUi(AppCtx& ctx) {
    TRY_VOID(m_GameScreen.OnRender(ctx));

    if (m_Paused) {
        TRY_VOID(m_PauseScreen.OnRender(ctx));
    }

    return Ok();
}

}  // namespace sop
