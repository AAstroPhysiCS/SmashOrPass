#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class GameModeSelectScreen : public UIScreen {
   public:
    GameModeSelectScreen(AppCtx& ctx);
    virtual ~GameModeSelectScreen() = default;

    void Build(UIBuilder& builder) final override;
};
}  // namespace sop
