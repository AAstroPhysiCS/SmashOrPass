#include "smashorpass/core/AppCtx.hpp"

namespace sop {

using namespace sop_util;

AppCtx::AppCtx() = default;

Result<void> AppCtx::Initialize() {
    TRY_VOID(m_Window.Initialize(
        WindowCreateInfo{.Width = 1920, .Height = 1080, .Title = "Smash Or Pass - The Game"}));
    TRY_VOID(m_Renderer.Initialize(m_Window));
    TRY_VOID(m_ParticleSystem.Initialize(m_Renderer));
    return Ok();
}

}  // namespace sop
