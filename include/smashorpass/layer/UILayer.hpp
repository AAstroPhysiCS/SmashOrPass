#pragma once

#include "smashorpass/layer/Layer.hpp"

namespace sop {
	class UILayer : public Layer {
	public:
		virtual ~UILayer() = default;

		void OnEvent(const Event& event) final override;
        void OnUpdate() final override;
        void OnRender() final override;
	};
}  // namespace sop