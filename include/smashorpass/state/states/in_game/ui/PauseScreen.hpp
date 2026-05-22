#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class PauseScreen : public UIScreen {
   public:
    explicit PauseScreen(AppCtx& ctx);

    void Build(UIBuilder& builder) override;
};

}  // namespace sop
