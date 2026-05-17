#pragma once

#include <memory>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/DisplayMetrics.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/Window.hpp"
#include "smashorpass/rendering/ParticleSystem.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/state/StateManager.hpp"
#include "smashorpass/util.hpp"

namespace sop {

using namespace sop_util;

struct AppCtx final {
    AppCtx();
    Result<void> Initialize();

    Window m_Window;
    Renderer m_Renderer;
    EventDispatcher m_EventDispatcher;
    ParticleSystem m_ParticleSystem;

    std::unique_ptr<AssetManager> Assets;
    StateManager m_StateManager;

    DisplayMetrics m_DisplayMetrics;

    bool RenderCollisionBoxes = false;
    bool DebugOverlayVisible = false;
    bool AppRunning = true;
};

}  // namespace sop
