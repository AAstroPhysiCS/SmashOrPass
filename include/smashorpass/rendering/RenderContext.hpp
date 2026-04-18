#pragma once

#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

	struct RenderContext final {
        Renderer& Renderer;
        // Add more stuff later here, any singleton-like rendering state that needs to be accessed
        // by multiple layers / components, etc.
	};
}