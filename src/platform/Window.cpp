#include "smashorpass/platform/Window.hpp"

#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL.h>
#include <stdexcept>

namespace sop {

    Window::Window(const WindowCreateInfo& createInfo) 
        : m_CreateInfo(createInfo) {

        SOP_SDL_ASSERT(
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD),
            SDL_GetError()
        );

        m_NativeHandle = SDL_CreateWindow(
            m_CreateInfo.Title.c_str(),
            m_CreateInfo.Width, 
            m_CreateInfo.Height,
            SDL_WINDOW_RESIZABLE
        );

        SOP_SDL_ASSERT(m_NativeHandle, SDL_GetError());
    }

    Window::~Window() {
        SDL_DestroyWindow(m_NativeHandle);
        m_NativeHandle = nullptr;

        SDL_Quit();
    }
}
