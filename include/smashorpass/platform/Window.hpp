#pragma once

#include <string>

struct SDL_Window;

namespace sop {

    struct WindowCreateInfo {
        int32_t Width{ 800 };
        int32_t Height{ 600 };
        std::string Title{"SmashOrPass"};
    };

    class Window {
    public:
        Window(const WindowCreateInfo& createInfo);
        ~Window();

        [[nodiscard]] inline SDL_Window *NativeHandle() const noexcept { return m_NativeHandle; }
      private:
        WindowCreateInfo m_CreateInfo;

        SDL_Window* m_NativeHandle;
    };
}
