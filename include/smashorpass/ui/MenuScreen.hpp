#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

	class MenuScreen : public UIScreen {
	public:
        MenuScreen(EventDispatcher& dispatcher);
		virtual ~MenuScreen() = default;

        void Build(UIBuilder& builder) final override;
	};
}
