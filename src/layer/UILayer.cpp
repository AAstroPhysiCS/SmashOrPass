#include "smashorpass/layer/UILayer.hpp"

#include "smashorpass/ui/MenuScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

UILayer::UILayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher)
    : Layer(renderer, window, eventDispatcher) {
    m_Screens.emplace_back(std::make_unique<MainMenuScreen>(eventDispatcher));
    // m_Screens.emplace_back(std::make_unique<GameOverScreen>(eventDispatcher));
    // m_Screens.emplace_back(std::make_unique<PauseScreen>(eventDispatcher));

    for (const auto& screen : m_Screens) {
        UIBuilder builder(*screen);
        screen->Build(builder);
    }
}

void UILayer::OnEvent(const Event& event) {
    for (const auto& component : m_Screens)
        component->OnEvent(event);
}

void UILayer::OnUpdate(ApplicationContext&) {
    for (const auto& component : m_Screens)
        component->OnUpdate();
}

void UILayer::OnRender(ApplicationContext&) {
    for (const auto& component : m_Screens)
        component->OnRender(GetRenderer());
}
}  // namespace sop
