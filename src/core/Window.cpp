#include "smashorpass/core/Window.hpp"

#include <SDL3/SDL.h>

#include <string>

#include "SDL3_ttf/SDL_ttf.h"

namespace sop {

using namespace sop_util;

Result<void> Window::Initialize(const WindowCreateInfo& createInfo) {
    m_CreateInfo = createInfo;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        return Err(SdlError("SDL_Init"));
    }
    m_SdlInitialized = true;

    if (!TTF_Init()) {
        SDL_Quit();
        m_SdlInitialized = false;
        return Err(SdlError("TTF_Init"));
    }
    m_TtfInitialized = true;

    m_NativeHandle = SDL_CreateWindow(m_CreateInfo.Title.c_str(),
                                      m_CreateInfo.Width,
                                      m_CreateInfo.Height,
                                      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (m_NativeHandle == nullptr) {
        const std::string error = SdlError("SDL_CreateWindow");
        TTF_Quit();
        SDL_Quit();
        m_TtfInitialized = false;
        m_SdlInitialized = false;
        return Err(error);
    }

    return Ok();
}

Window::~Window() {
    SDL_DestroyWindow(m_NativeHandle);
    m_NativeHandle = nullptr;

    if (m_TtfInitialized) {
        TTF_Quit();
        m_TtfInitialized = false;
    }

    if (m_SdlInitialized) {
        SDL_Quit();
        m_SdlInitialized = false;
    }
}

Result<SDL_Point> Window::GetSize() const {
    SDL_Point size{};
    const bool ok = SDL_GetWindowSize(m_NativeHandle, &size.x, &size.y);
    if (!ok) {
        return Err(SdlError("SDL_GetWindowSize"));
    }
    return Ok(size);
}

Result<SDL_Point> Window::GetPixelSize() const {
    SDL_Point size{};
    const bool ok = SDL_GetWindowSizeInPixels(m_NativeHandle, &size.x, &size.y);
    if (!ok) {
        return Err(SdlError("SDL_GetWindowSizeInPixels"));
    }
    return Ok(size);
}

Result<float> Window::GetDisplayScale() const {
    const float scale = SDL_GetWindowDisplayScale(m_NativeHandle);
    if (scale <= 0.0f) {
        return Err(SdlError("SDL_GetWindowDisplayScale"));
    }
    return Ok(NormalizeDisplayScale(scale));
}

Result<float> Window::GetPixelDensity() const {
    const float density = SDL_GetWindowPixelDensity(m_NativeHandle);
    if (density <= 0.0f) {
        return Err(SdlError("SDL_GetWindowPixelDensity"));
    }
    return Ok(NormalizeDisplayScale(density));
}

Result<DisplayMetrics> Window::GetDisplayMetrics() const {
    TRY(windowSize, GetSize());
    TRY(pixelSize, GetPixelSize());
    TRY(displayScale, GetDisplayScale());
    TRY(pixelDensity, GetPixelDensity());

    return Ok(DisplayMetrics{
        .WindowSize = windowSize,
        .PixelSize = pixelSize,
        .DisplayScale = displayScale,
        .PixelDensity = pixelDensity,
    });
}
}  // namespace sop
