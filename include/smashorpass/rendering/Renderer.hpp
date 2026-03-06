#pragma once

#include "smashorpass/platform/Window.hpp"

namespace sop {

    class Renderer final {
    public:
        explicit Renderer(Window& window);

        void BeginFrame();
        void EndFrame();
    private:
        Window &m_Window;
    };
}
