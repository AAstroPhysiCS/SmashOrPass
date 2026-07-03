#pragma once

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class KeybindSettingsScreen : public UIScreen {
   public:
    explicit KeybindSettingsScreen(AppCtx& ctx);
    ~KeybindSettingsScreen() override = default;

    void Build(UIBuilder& builder) final override;
};

}  // namespace sop
