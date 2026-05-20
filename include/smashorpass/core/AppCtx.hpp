#pragma once

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/DisplayMetrics.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/InputHelper.hpp"
#include "smashorpass/core/Window.hpp"
#include "smashorpass/rendering/ParticleSystem.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/state/StateManager.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx final {
    AppCtx();
    Result<void> Initialize();

    Window Window;
    Renderer Renderer;
    EventDispatcher EventDispatcher;
    Input Input;
    ParticleSystem ParticleSystem;

    AssetManager Assets;
    StateManager StateManager;

    DisplayMetrics DisplayMetrics;

    bool RenderCollisionBoxes = false;
    bool DebugOverlayVisible = false;
    bool AppRunning = true;
};

}  // namespace sop
