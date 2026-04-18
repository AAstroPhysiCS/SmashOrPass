#pragma once

#include "smashorpass/layer/Layer.hpp"

#include "smashorpass/core/Base.hpp"

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

	class UILayer : public Layer {
	public:
        UILayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher);
		virtual ~UILayer() = default;

		void OnEvent(const Event& event) final override;
        void OnUpdate() final override;
        void OnRender() final override;
	private:
        std::vector<std::unique_ptr<UIScreen>> m_Screens;
	};
}  // namespace sop