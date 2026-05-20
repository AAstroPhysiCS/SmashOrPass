#include "smashorpass/core/AppCtx.hpp"

namespace sop {

AppCtx::AppCtx() : Assets(SOP_ASSET_ROOT_DIR) {}

Result<void> AppCtx::Initialize() {
    TRY_VOID(Window.Initialize(
        WindowCreateInfo{.Width = 1920, .Height = 1080, .Title = "Smash Or Pass - The Game"}));
    TRY_VOID(Renderer.Initialize(Window));
    TRY_VOID(ParticleSystem.Initialize(Renderer));
    return Ok();
}

}  // namespace sop
