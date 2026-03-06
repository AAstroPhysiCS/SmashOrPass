#include "smashorpass/platform/Window.hpp"

#include "smashorpass/core/Base.hpp"

#include <SDL.h>
#include <stdexcept>

namespace sop {

    Window::Window(const WindowCreateInfo& createInfo) 
        : m_CreateInfo(createInfo) {

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
            SOP_SDL_ASSERT(false, SDL_GetError());

        m_NativeHandle = SDL_CreateWindow(
            m_CreateInfo.Title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, m_CreateInfo.Width, m_CreateInfo.Height,
            SDL_WINDOW_SHOWN
        );

        SOP_SDL_ASSERT(m_NativeHandle, SDL_GetError());
    }

    Window::~Window() {
        if (m_NativeHandle == nullptr)
            return;

        SDL_DestroyWindow(m_NativeHandle);
        SDL_Quit();
    }
}
