#include "smashorpass/state/states/in_game/InGameState.hpp"

#include <chrono>
#include <string>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

using Clock = std::chrono::steady_clock;

namespace sop {

using namespace sop_util;

constexpr int kGameLogicTicksPerSecond = 120;
constexpr int kGameLogicMaxCatchUpTicks = 10;
constexpr int kAnimationTicksPerSecond = 60;
constexpr int kAnimationMaxCatchUpTicks = 10;
// Derived from above
constexpr Clock::duration kGameLogicTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kGameLogicTicksPerSecond));
constexpr Clock::duration kAnimationTickDuration =
    duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / kAnimationTicksPerSecond));

InGameState::InGameState(AppCtx& ctx) : m_GameScreen(ctx), m_PauseScreen(ctx) {
    UIBuilder gameScreenBuilder(m_GameScreen);
    m_GameScreen.Build(gameScreenBuilder);

    UIBuilder pauseScreenBuilder(m_PauseScreen);
    m_PauseScreen.Build(pauseScreenBuilder);

    m_Game.SetDisplayMetrics(ctx.m_DisplayMetrics);
    ResetFrameTimer();
}

Result<EventFlow> InGameState::OnEvent(AppCtx& ctx, const Event& event) {
    if (const auto* navigation = std::get_if<NavigationEvent>(&event.Payload)) {
        if (navigation->Action == NavigationAction::ResumeMatch) {
            m_Paused = false;
            ResetFrameTimer();
            return Ok(EventFlow::Consumed);
        }
        return Ok(EventFlow::Passed);
    }

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat && keyEvent->Key == SDLK_ESCAPE) {
            TogglePause();
            return Ok(EventFlow::Consumed);
        }
    }

    if (m_Paused) {
        const EventFlow pauseUiFlow = m_PauseScreen.OnEvent(ctx, event);
        if (pauseUiFlow == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }
        return Ok(EventFlow::Passed);
    }

    const EventFlow gameUiFlow = m_GameScreen.OnEvent(ctx, event);
    if (gameUiFlow == EventFlow::Consumed) {
        return Ok(EventFlow::Consumed);
    }

    m_Game.OnEvent(ctx, event);
    return Ok(EventFlow::Passed);
}

Result<void> InGameState::OnUpdate(AppCtx& ctx) {
    const Clock::time_point now = Clock::now();
    const Clock::duration elapsed = now - m_PreviousUpdateTime;
    m_PreviousUpdateTime = now;
    const float dt = std::chrono::duration<float>(elapsed).count();

    if (m_Paused) {
        m_PauseScreen.OnUpdate(ctx);
        return Ok();
    }

    // ---- Update Ticks
    // Game Logic
    int gameLogicTicks = 0;
    while (now - m_PreviousGameLogicTick >= kGameLogicTickDuration &&
           gameLogicTicks < kGameLogicMaxCatchUpTicks) {
        TRY_VOID(m_Game.GameplayTick(ctx, kGameLogicTickDuration));
        m_PreviousGameLogicTick += kGameLogicTickDuration;
        ++gameLogicTicks;
    }
    if (gameLogicTicks == kGameLogicMaxCatchUpTicks) {
        // We are too far behind, drop backlog.
        m_PreviousGameLogicTick = now;
    }

    // Animations
    int animationTicks = 0;
    while (now - m_PreviousAnimationTick >= kAnimationTickDuration &&
           animationTicks < kAnimationMaxCatchUpTicks) {
        TRY_VOID(m_Game.AnimationTick(ctx));
        m_PreviousAnimationTick += kAnimationTickDuration;
        ++animationTicks;
    }
    if (animationTicks == kAnimationMaxCatchUpTicks) {
        // We are too far behind, drop backlog.
        m_PreviousAnimationTick = now;
    }

    m_GameScreen.OnUpdate(ctx);
    ctx.m_ParticleSystem.Update(dt);

    return Ok();
}

Result<void> InGameState::OnRender(AppCtx& ctx) {
    if (ctx.Assets == nullptr) {
        return Err(std::string("Application context missing asset manager"));
    }

    m_Game.SetDisplayMetrics(ctx.m_DisplayMetrics);
    TRY_VOID(m_Game.Render(ctx));
    TRY_VOID(ctx.m_ParticleSystem.Render(ctx));
    TRY_VOID(m_GameScreen.OnRender(ctx));

    if (m_Paused) {
        TRY_VOID(m_PauseScreen.OnRender(ctx));
    }

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

}  // namespace sop
