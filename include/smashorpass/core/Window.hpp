#pragma once

#include <SDL3/SDL_rect.h>

#include <string>

#include "smashorpass/core/DisplayMetrics.hpp"
#include "smashorpass/util.hpp"

struct SDL_Window;

namespace sop {

using namespace sop_util;

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

    [[nodiscard]] Result<void> Initialize(const WindowCreateInfo& createInfo);

    [[nodiscard]] Result<SDL_Point> GetSize() const;
    [[nodiscard]] Result<SDL_Point> GetPixelSize() const;
    [[nodiscard]] Result<float> GetDisplayScale() const;
    [[nodiscard]] Result<float> GetPixelDensity() const;
    [[nodiscard]] Result<DisplayMetrics> GetDisplayMetrics() const;

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
