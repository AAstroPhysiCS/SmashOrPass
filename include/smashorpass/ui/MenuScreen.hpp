#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

	class MenuScreen : public UIScreen {
	public:
		virtual ~MenuScreen() = default;

        void Build(UIBuilder& builder) final override;

		void OnEvent(const Event& event) final override;
	};
}
