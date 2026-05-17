#pragma once

#include <string_view>
#include <utility>

#include "smashorpass/app/AppCtx.hpp"
using namespace sop_util;

namespace sop {

class Application {
   public:
    Application();
    ~Application();

    template <typename TOverlay, typename... TArgs>
    inline TOverlay& PushOverlay(TArgs&&... args) {
        return ctx.m_StateManager.PushOverlay<TOverlay>(ctx, std::forward<TArgs>(args)...);
    }

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
};
}  // namespace sop
