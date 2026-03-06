#include "smashorpass/debug/DebugOverlay.hpp"

#ifdef SOP_ENABLE_IMGUI_DEMO
#include <imgui.h>
#endif

namespace sop {
    void DebugOverlay::Draw() {
#ifdef SOP_ENABLE_IMGUI_DEMO
            ImGui::ShowDemoWindow();
#endif
    }
} // namespace sop
