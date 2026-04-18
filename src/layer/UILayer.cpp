#include "smashorpass/layer/UILayer.hpp"

#include "smashorpass/rendering/RenderContext.hpp"

#include "smashorpass/ui/MenuScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

	UILayer::UILayer(Renderer& renderer, const Window& window) 
		: Layer(renderer, window) {
		m_Screens.emplace_back(std::make_unique<MenuScreen>());

		for (const auto& screen : m_Screens) {
			UIBuilder builder(*screen);
			screen->Build(builder);
		}
	}

	void UILayer::OnEvent(const Event& event) {
		for (const auto& component : m_Screens)
			component->OnEvent(event);
	}

	void UILayer::OnUpdate() {
        for (const auto& component : m_Screens)
            component->OnUpdate();
	}

	void UILayer::OnRender() {
        for (const auto& component : m_Screens)
            component->OnRender(GetRenderer());
	}
}  // namespace sop