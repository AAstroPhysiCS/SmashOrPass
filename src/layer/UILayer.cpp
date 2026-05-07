#include "smashorpass/layer/UILayer.hpp"

#include "smashorpass/ui/MenuScreen.hpp"
#include "smashorpass/ui/CharacterSelectScreen.hpp"
#include "smashorpass/ui/PauseScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

UILayer::UILayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher)
    : Layer(renderer, window, eventDispatcher) {
    m_Screens.emplace_back(std::make_unique<MainMenuScreen>(eventDispatcher));
    m_Screens.emplace_back(std::make_unique<CharacterSelectScreen>(eventDispatcher));
    m_Screens.emplace_back(std::make_unique<PauseScreen>(eventDispatcher));
    // m_Screens.emplace_back(std::make_unique<GameOverScreen>(eventDispatcher));

    for (const auto& screen : m_Screens) {
        UIBuilder builder(*screen);
        screen->Build(builder);
    }
}

void UILayer::OnEvent(const Event& event, ApplicationContext& ctx) {
    for (const auto& component : m_Screens)
        if (component->GetApplicationState() == ctx.CurrentState)
            component->OnEvent(event);
}

void UILayer::OnUpdate(ApplicationContext& ctx) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState)
            component->OnUpdate();
    }
}

void UILayer::OnRender(ApplicationContext& ctx) {
    for (const auto& component : m_Screens) {
        if (component->GetApplicationState() == ctx.CurrentState)
            component->OnRender(GetRenderer());
    }
}
}  // namespace sop
