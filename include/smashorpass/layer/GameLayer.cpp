#include "GameLayer.hpp"

namespace sop {

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