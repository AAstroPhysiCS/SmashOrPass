#pragma once

#include <cstdint>
#include <memory>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/ApplicationState.hpp"
#include "smashorpass/core/DisplayMetrics.hpp"

namespace sop {

struct ApplicationContext final {
    ApplicationState CurrentState = ApplicationState::MainMenu;
    std::unique_ptr<AssetManager> Assets;
    uint64_t GameplayTickCount = 0;
    uint64_t AnimationTickCount = 0;
    double GameplayStepSeconds = 1.0 / 120.0;
    double AnimationStepSeconds = 1.0 / 60.0;
    DisplayMetrics Display{};
    bool RenderCollisionBoxes = false;
    // - AudioSystem AudioSystem;
    // Add more stuff later here, any singleton-like stuff that needs to be accessed globally by
    // multiple layers / components etc. can go here, but try to avoid this as much as possible and
    // prefer passing things explicitly
};
}  // namespace sop
