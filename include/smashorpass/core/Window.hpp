#pragma once

#include <SDL3/SDL_rect.h>

#include <string>

#include "smashorpass/core/DisplayMetrics.hpp"
#include "smashorpass/util.hpp"

struct SDL_Window;

namespace sop {

struct WindowCreateInfo {
    int32_t Width{800};
    int32_t Height{600};
    std::string Title{"SmashOrPass"};
};

class Window {
   public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    Result<void> Initialize(const WindowCreateInfo& createInfo);

    Result<SDL_Point> GetSize() const;
    Result<SDL_Point> GetPixelSize() const;
    Result<float> GetDisplayScale() const;
    Result<float> GetPixelDensity() const;
    Result<DisplayMetrics> GetDisplayMetrics() const;

    [[nodiscard]] inline SDL_Window* NativeHandle() const noexcept {
        return m_NativeHandle;
    }

   private:
    WindowCreateInfo m_CreateInfo;

    SDL_Window* m_NativeHandle{nullptr};
    bool m_SdlInitialized{false};
    bool m_TtfInitialized{false};
};
}  // namespace sop
