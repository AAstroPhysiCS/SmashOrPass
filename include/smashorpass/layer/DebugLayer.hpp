#pragma once

#include "Layer.hpp"
#include "SDL3/SDL.h"

namespace sop {

class Window;
class Renderer;

class DebugLayer : public Layer {
   public:
    DebugLayer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher);
    ~DebugLayer();

    void BeginFrame();
    void Draw(ApplicationContext& ctx);
    void EndFrame();

    void OnEvent(const Event& event, ApplicationContext& ctx) final override;
    void OnUpdate(ApplicationContext& ctx) final override;
    void OnRender(ApplicationContext& ctx) final override;
};
}  // namespace sop
