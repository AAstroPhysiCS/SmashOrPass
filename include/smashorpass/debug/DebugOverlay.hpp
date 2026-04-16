#pragma once

#include "SDL3/SDL.h"

namespace sop {

    class Window;
    class Renderer;

    class DebugOverlay {
    public:
        DebugOverlay(Window& window, Renderer& renderer);
        ~DebugOverlay();

        void OnEvent(const SDL_Event& event);
        void BeginFrame();
        void Draw();
        void EndFrame();
    private:
        Window& m_Window;
        Renderer& m_Renderer;
    };
}