#include "smashorpass/state/states/InGameState.hpp"

#include <algorithm>

#include "smashorpass/app/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {
namespace {

constexpr double kGameplayStepSeconds = 1.0 / 120.0;
constexpr double kAnimationStepSeconds = 1.0 / 60.0;
constexpr double kMaxFrameDeltaSeconds = 0.25;

}  // namespace

InGameState::InGameState(AppCtx& ctx)
    : m_GameScreen(ctx.m_EventDispatcher),
      m_PauseScreen(ctx.m_EventDispatcher) {
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
        const EventFlow pauseUiFlow = m_PauseScreen.OnEvent(event);
        if (pauseUiFlow == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }

        m_Game.OnEvent(ctx, event);
        return Ok(EventFlow::Passed);
    }

    const EventFlow gameUiFlow = m_GameScreen.OnEvent(event);
    if (gameUiFlow == EventFlow::Consumed) {
        return Ok(EventFlow::Consumed);
    }

    m_Game.OnEvent(ctx, event);
    return Ok(EventFlow::Passed);
}

Result<void> InGameState::OnUpdate(AppCtx& ctx) {
    const Clock::time_point currentTime = Clock::now();
    const double elapsedSeconds =
        std::chrono::duration<double>(currentTime - m_PreviousUpdateTime).count();
    m_PreviousUpdateTime = currentTime;

    if (m_Paused) {
        m_PauseScreen.OnUpdate();
        return Ok();
    }

    const double clampedElapsedSeconds =
        std::clamp(elapsedSeconds, 0.0, kMaxFrameDeltaSeconds);
    auto result = TickGame(ctx, clampedElapsedSeconds);
    if (!result) {
        return result;
    }

    m_GameScreen.OnUpdate();
    ctx.m_ParticleSystem.Update(static_cast<float>(clampedElapsedSeconds));
    return Ok();
}

Result<void> InGameState::OnRender(AppCtx& ctx) {
    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");

    m_Game.SetDisplayMetrics(ctx.m_DisplayMetrics);
    m_Game.Render(ctx.m_Renderer, ctx.m_EventDispatcher, *ctx.Assets, ctx.RenderCollisionBoxes);
    ctx.m_ParticleSystem.Render(ctx.m_Renderer);
    m_GameScreen.OnRender(ctx.m_Renderer);

    if (m_Paused) {
        m_PauseScreen.OnRender(ctx.m_Renderer);
    }

    return Ok();
}

void InGameState::ResetFrameTimer() {
    m_PreviousUpdateTime = Clock::now();
}

void InGameState::TogglePause() {
    m_Paused = !m_Paused;
    ResetFrameTimer();
}

Result<void> InGameState::TickGame(AppCtx& ctx, double elapsedSeconds) {
    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");

    m_GameplayAccumulatorSeconds += elapsedSeconds;
    while (m_GameplayAccumulatorSeconds >= kGameplayStepSeconds) {
        m_GameplayAccumulatorSeconds -= kGameplayStepSeconds;
        m_Game.GameplayTick(kGameplayStepSeconds, *ctx.Assets, ctx.m_ParticleSystem);
    }

    m_AnimationAccumulatorSeconds += elapsedSeconds;
    while (m_AnimationAccumulatorSeconds >= kAnimationStepSeconds) {
        m_AnimationAccumulatorSeconds -= kAnimationStepSeconds;
        m_Game.AnimationTick(*ctx.Assets);
    }

    return Ok();
}

}  // namespace sop
