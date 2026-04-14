#include "smashorpass/rendering/Renderer.hpp"

#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL.h>

namespace sop {

    Renderer::Renderer(Window& window)
        : m_Window(window) {
        m_NativeHandle = SDL_CreateRenderer(m_Window.nativeHandle(), nullptr);
        SOP_SDL_ASSERT(m_NativeHandle != nullptr, SDL_GetError());
    }

    Renderer::~Renderer() {
        if (m_NativeHandle == nullptr)
            return;

        SDL_DestroyRenderer(m_NativeHandle);
        m_NativeHandle = nullptr;
    }

    void Renderer::BeginFrame() {
        SOP_VERIFY(SDL_SetRenderDrawColor(m_NativeHandle, 18, 18, 18, 255), SDL_GetError());
        SOP_VERIFY(SDL_RenderClear(m_NativeHandle), SDL_GetError());
    }

    void Renderer::EndFrame() {
        SOP_VERIFY(SDL_RenderPresent(m_NativeHandle), SDL_GetError());
    }
}