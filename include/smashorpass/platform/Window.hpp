#pragma once

#include <SDL3/SDL_rect.h>

#include <string>

#include "smashorpass/core/DisplayMetrics.hpp"

struct SDL_Window;

namespace sop {

struct WindowCreateInfo {
    int32_t Width{800};
    int32_t Height{600};
    std::string Title{"SmashOrPass"};
};

class Window {
   public:
    Window(const WindowCreateInfo& createInfo);
    ~Window();

    [[nodiscard]] SDL_Point GetSize() const;
    [[nodiscard]] SDL_Point GetPixelSize() const;
    [[nodiscard]] float GetDisplayScale() const;
    [[nodiscard]] float GetPixelDensity() const;
    [[nodiscard]] DisplayMetrics GetDisplayMetrics() const;

    [[nodiscard]] inline SDL_Window* NativeHandle() const noexcept {
        return m_NativeHandle;
    }

   private:
    WindowCreateInfo m_CreateInfo;

    SDL_Window* m_NativeHandle{nullptr};
};
}  // namespace sop
