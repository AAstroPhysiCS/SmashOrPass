#include "smashorpass/state/GameState.hpp"

#include "smashorpass/app/AppCtx.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/ui/GameScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

GameState::GameState(AppCtx& ctx) {
    m_Screens.emplace_back(std::make_unique<GameScreen>(ctx.m_EventDispatcher));

    for (const auto& screen : m_Screens) {
        UIBuilder builder(*screen);
        screen->Build(builder);
    }
}

Result<EventFlow> GameState::OnEvent(AppCtx& ctx, const Event& event) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState &&
            component->OnEvent(event) == EventFlow::Consumed) {
            return Ok(EventFlow::Consumed);
        }
    }

    m_Game.OnEvent(ctx, event);
    return Ok(EventFlow::Passed);
}

Result<void> GameState::OnUpdate(AppCtx& ctx) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState) {
            component->OnUpdate();
        }
    }

    return Ok();
}

Result<void> GameState::OnRender(AppCtx& ctx) {
    SOP_ASSERT(ctx.Assets != nullptr, "Application context missing asset manager");

    m_Game.SetDisplayMetrics(ctx.Display);
    m_Game.Render(ApplicationState::Playing,
                  ctx.m_Renderer,
                  ctx.m_EventDispatcher,
                  *ctx.Assets,
                  ctx.RenderCollisionBoxes);
    ctx.m_ParticleSystem.Render(ctx.m_Renderer);

    if (ctx.CurrentState == ApplicationState::Playing) {
        for (const auto& component : m_Screens) {
            if (component->GetApplicationState() == ctx.CurrentState) {
                component->OnRender(ctx.m_Renderer);
            }
        }
    }

    return Ok();
}

}  // namespace sop
