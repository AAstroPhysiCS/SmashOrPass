#pragma once

#include "smashorpass/core/Event.hpp"

namespace sop {

	struct Layer {
		virtual ~Layer() = default;

		virtual void OnEvent(const Event& event) = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;
	};

    template <typename TLayer>
    concept IsLayer = std::is_base_of_v<Layer, TLayer>;
} // namespace sop