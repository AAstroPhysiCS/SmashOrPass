#pragma once

#include <memory>
#include <vector>

#include "smashorpass/state/State.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class UIState : public State {
   public:
    explicit UIState(AppCtx& ctx);
    ~UIState() override = default;

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

   private:
    std::vector<std::unique_ptr<UIScreen>> m_Screens;
};

}  // namespace sop
