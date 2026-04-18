#include "smashorpass/layer/GameLayer.hpp"

#include "smashorpass/ui/GameScreen.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {
	
	GameLayer::GameLayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher) 
		: Layer(renderer, window, eventDispatcher) 
	{
		m_Screens.emplace_back(std::make_unique<GameScreen>(eventDispatcher));

		for (const auto& screen : m_Screens) {
			UIBuilder builder(*screen);
			screen->Build(builder);
		}
	}

	void GameLayer::OnEvent(const Event& event) 
	{
        for (const auto& component : m_Screens)
            component->OnEvent(event);
		m_Game.OnEvent(event);
	}

	void GameLayer::OnUpdate() 
	{
        for (const auto& component : m_Screens)
            component->OnUpdate();
        m_Game.Update();
	}

	void GameLayer::OnRender() 
	{
        auto& renderer = GetRenderer();
        for (const auto& component : m_Screens)
            component->OnRender(renderer);
        m_Game.Render(renderer);
	}
}  // namespace sop