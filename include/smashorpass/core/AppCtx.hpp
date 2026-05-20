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

    Window window;
    Renderer renderer;
    EventDispatcher eventDispatcher;
    Input input;
    ParticleSystem particleSystem;

    AssetManager assets;
    StateManager stateManager;

    DisplayMetrics displayMetrics;

    bool renderCollisionBoxes = false;
    bool debugOverlayVisible = false;
    bool appRunning = true;
};

}  // namespace sop
