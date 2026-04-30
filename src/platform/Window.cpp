#include "smashorpass/platform/Window.hpp"

#include <SDL3/SDL.h>

#include <stdexcept>

#include "smashorpass/core/Base.hpp"

namespace sop {

Window::Window(const WindowCreateInfo& createInfo) : m_CreateInfo(createInfo) {
    const bool initialized = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
    SOP_SDL_ASSERT(initialized, SDL_GetError());
    if (!initialized) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    m_NativeHandle = SDL_CreateWindow(m_CreateInfo.Title.c_str(),
                                      m_CreateInfo.Width,
                                      m_CreateInfo.Height,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    SOP_SDL_ASSERT(m_NativeHandle, SDL_GetError());
    if (m_NativeHandle == nullptr) {
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }
}

Window::~Window() {
    SDL_DestroyWindow(m_NativeHandle);
    m_NativeHandle = nullptr;

    SDL_Quit();
}

SDL_Point Window::GetSize() const {
    SDL_Point size{};
    const bool ok = SDL_GetWindowSize(m_NativeHandle, &size.x, &size.y);
    SOP_SDL_ASSERT(ok, SDL_GetError());
    return ok ? size : SDL_Point{};
}

SDL_Point Window::GetPixelSize() const {
    SDL_Point size{};
    const bool ok = SDL_GetWindowSizeInPixels(m_NativeHandle, &size.x, &size.y);
    SOP_SDL_ASSERT(ok, SDL_GetError());
    return ok ? size : SDL_Point{};
}

float Window::GetDisplayScale() const {
    const float scale = SDL_GetWindowDisplayScale(m_NativeHandle);
    SOP_SDL_ASSERT(scale > 0.0f, SDL_GetError());
    return NormalizeDisplayScale(scale);
}

float Window::GetPixelDensity() const {
    const float density = SDL_GetWindowPixelDensity(m_NativeHandle);
    SOP_SDL_ASSERT(density > 0.0f, SDL_GetError());
    return NormalizeDisplayScale(density);
}

DisplayMetrics Window::GetDisplayMetrics() const {
    return DisplayMetrics{
        .WindowSize = GetSize(),
        .PixelSize = GetPixelSize(),
        .DisplayScale = GetDisplayScale(),
        .PixelDensity = GetPixelDensity(),
    };
}
}  // namespace sop
