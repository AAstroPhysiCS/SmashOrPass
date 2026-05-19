#pragma once

#include <string_view>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

class Application {
   public:
    Application();
    ~Application();

    sop_util::Result<void> Initialize();
    sop_util::Result<void> Run();

   private:
    sop_util::Result<void> ProcessEvents();
    sop_util::Result<void> Update();
    sop_util::Result<void> Render();

    sop_util::Result<void> DispatchEvent(const Event& event);
    sop_util::Result<void> RefreshDisplayMetrics();
    sop_util::Result<void> OnEvent(const Event& event);
    sop_util::Result<void> ToggleDebugOverlay();

    AppCtx ctx;
    bool m_Initialized{false};
};
}  // namespace sop
