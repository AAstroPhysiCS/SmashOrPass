#include "smashorpass/state/UIState.hpp"

#include "smashorpass/app/AppCtx.hpp"
#include "smashorpass/ui/CharacterSelectScreen.hpp"
#include "smashorpass/ui/MenuScreen.hpp"
#include "smashorpass/ui/PauseScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

UIState::UIState(AppCtx& ctx) {
    m_Screens.emplace_back(std::make_unique<MainMenuScreen>(ctx.m_EventDispatcher));
    m_Screens.emplace_back(std::make_unique<CharacterSelectScreen>(ctx.m_EventDispatcher));
    m_Screens.emplace_back(std::make_unique<PauseScreen>(ctx.m_EventDispatcher));

    for (const auto& screen : m_Screens) {
        UIBuilder builder(*screen);
        screen->Build(builder);
    }
}

EventFlow UIState::OnEvent(AppCtx& ctx, const Event& event) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState) {
            return component->OnEvent(event);
        }
    }

    return EventFlow::Passed;
}

Result<void> UIState::OnUpdate(AppCtx& ctx) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState) {
            component->OnUpdate();
        }
    }

    return sop_util::Ok();
}

Result<void> UIState::OnRender(AppCtx& ctx) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState) {
            component->OnRender(ctx.m_Renderer);
        }
    }

    return sop_util::Ok();
}

}  // namespace sop
