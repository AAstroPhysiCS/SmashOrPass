#pragma once

#include "smashorpass/platform/Window.hpp"

struct SDL_Renderer;

namespace sop {

    class Renderer final {
    public:
        explicit Renderer(Window& window);
        ~Renderer();

        void BeginFrame();
        void EndFrame();

        [[nodiscard]] inline SDL_Renderer* nativeHandle() const { return m_NativeHandle; }
    private:
        Window& m_Window;

        SDL_Renderer* m_NativeHandle{ nullptr };
    };
}
