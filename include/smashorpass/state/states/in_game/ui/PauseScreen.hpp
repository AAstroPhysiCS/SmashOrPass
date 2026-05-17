#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class PauseScreen : public UIScreen {
   public:
    explicit PauseScreen(EventDispatcher& dispatcher);

    void Build(UIBuilder& builder) override;
};

}  // namespace sop