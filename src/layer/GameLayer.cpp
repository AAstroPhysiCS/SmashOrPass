#include "smashorpass/layer/GameLayer.hpp"

namespace sop {
	
	GameLayer::GameLayer(Renderer& renderer, const Window& window) 
		: Layer(renderer, window) 
	{
	}

	void GameLayer::OnEvent(const Event& event) 
	{
		m_Game.OnEvent(event);
	}

	void GameLayer::OnUpdate() 
	{
        m_Game.Update();
	}

	void GameLayer::OnRender() 
	{

	}
}  // namespace sop