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
        void Draw();
        void EndFrame();
        
        void OnEvent(const Event& event) final override;
        void OnUpdate() final override;
        void OnRender() final override;
    };
}