#pragma once

#include <string_view>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "spdlog/spdlog.h"  // in order to have spdlog in entrypoint... dont want the "client side" to have to include spdlog just for logging in main()

namespace sop {

class Application {
   public:
    Application();
    ~Application() = default;

    Result<void> Run();

   private:
    Result<void> ProcessEvents();
    Result<void> Update();
    Result<void> Render();

    Result<void> DispatchEvent(const Event& event);
    Result<void> RefreshDisplayMetrics();
    Result<void> OnEvent(const Event& event);
    Result<void> ToggleDebugOverlay();

    AppCtx ctx;
    bool m_Initialized{false};
};
}  // namespace sop
