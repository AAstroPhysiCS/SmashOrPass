#pragma once

#include "Layer.hpp"

#include "SDL3/SDL.h"

namespace sop {

    class Window;
    class Renderer;

    class DebugLayer : public Layer {
    public:
        DebugLayer(Window& window, Renderer& renderer);
        ~DebugLayer();

        void OnEvent(const SDL_Event& event);
        void BeginFrame();
        void Draw();
        void EndFrame();
        
        void OnEvent(const Event& event) final override;
        void OnUpdate() final override;
        void OnRender() final override;
    private:
        Window& m_Window;
        Renderer& m_Renderer;
    };
}