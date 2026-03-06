#pragma once

#include <memory>

#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/debug/DebugOverlay.hpp"
#include "smashorpass/core/Game.hpp"

namespace sop {

    class Application {
    public:
        Application();
        ~Application() = default;

        int Run();
    private:
        void ProcessEvents(bool& running);
        void Update();
        void Render();

        /* The order is important! */
        Window m_Window;
        Renderer m_Renderer;
        DebugOverlay m_DebugOverlay;
        Game m_Game;
    };
}
