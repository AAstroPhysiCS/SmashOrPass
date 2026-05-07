#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class MainMenuScreen : public UIScreen {
   public:
    MainMenuScreen(EventDispatcher& dispatcher);
    virtual ~MainMenuScreen() = default;

    void Build(UIBuilder& builder) final override;
};
}  // namespace sop
