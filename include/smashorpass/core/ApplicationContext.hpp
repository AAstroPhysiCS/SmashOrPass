#pragma once

#include <memory>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/ApplicationState.hpp"

namespace sop {

	struct ApplicationContext final {
        ApplicationState CurrentState = ApplicationState::MainMenu;
        std::unique_ptr<AssetManager> Assets;
        // - AudioSystem AudioSystem;
        // Add more stuff later here, any singleton-like stuff that needs to be accessed globally by multiple layers / components etc.
        // can go here, but try to avoid this as much as possible and prefer passing things explicitly
	};
}
