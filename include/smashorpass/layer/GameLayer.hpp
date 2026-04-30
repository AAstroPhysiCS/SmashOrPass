#pragma once

#include <memory>

#include "smashorpass/core/Game.hpp"
#include "smashorpass/layer/Layer.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class GameLayer : public Layer {
   public:
    GameLayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher);
    virtual ~GameLayer() {}

    void OnEvent(const Event& event) final override;
    void OnGameplayTick(ApplicationContext& ctx) final override;
    void OnAnimationTick(ApplicationContext& ctx) final override;
    void OnUpdate(ApplicationContext& ctx) final override;
    void OnRender(ApplicationContext& ctx) final override;

   private:
    Game m_Game;

    std::vector<std::unique_ptr<UIScreen>> m_Screens;
};
}  // namespace sop
