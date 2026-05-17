#pragma once

#include <memory>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/Game.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/ui/UIScreen.hpp"
#include "smashorpass/util.hpp"
using namespace sop_util;

namespace sop {

class GameState : public State {
   public:
    GameState(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher);
    virtual ~GameState() {}

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final override;
    Result<void> OnUpdate(AppCtx& ctx) final override;
    Result<void> OnRender(AppCtx& ctx) final override;

   private:
    Game m_Game;
    std::vector<std::unique_ptr<UIScreen>> m_Screens;
};
}  // namespace sop
