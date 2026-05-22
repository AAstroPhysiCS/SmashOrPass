#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class MainMenuScreen : public UIScreen {
   public:
    MainMenuScreen(AppCtx& ctx);
    virtual ~MainMenuScreen() = default;

    void Build(UIBuilder& builder) final override;
};
}  // namespace sop
