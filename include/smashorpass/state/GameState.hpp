#pragma once

#include <memory>
#include <vector>

#include "smashorpass/core/Game.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class GameState : public State {
   public:
    explicit GameState(AppCtx& ctx);
    ~GameState() override = default;

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

   private:
    Game m_Game;
    std::vector<std::unique_ptr<UIScreen>> m_Screens;
};

}  // namespace sop
