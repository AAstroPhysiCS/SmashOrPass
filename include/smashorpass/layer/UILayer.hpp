#pragma once

#include <memory>

#include "smashorpass/core/Base.hpp"
#include "smashorpass/layer/Layer.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class UILayer : public Layer {
   public:
    UILayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher);
    virtual ~UILayer() = default;

    void OnEvent(const Event& event) final override;
    void OnUpdate(ApplicationContext& ctx) final override;
    void OnRender(ApplicationContext& ctx) final override;

   private:
    std::vector<std::unique_ptr<UIScreen>> m_Screens;
};
}  // namespace sop
