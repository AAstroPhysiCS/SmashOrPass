#pragma once

#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

#include "smashorpass/core/Event.hpp"

namespace sop {

	struct Layer {
        Layer(Renderer& renderer, const Window& window) 
            : m_Renderer(renderer), m_Window(window) {}
		virtual ~Layer() = default;

		virtual void OnEvent(const Event& event) = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;

        [[nodiscard]] inline Renderer& GetRenderer() { return m_Renderer; }
        [[nodiscard]] inline const Window& GetWindow() const { return m_Window; }
    private:
        Renderer& m_Renderer;
        const Window& m_Window;
	};

    template <typename TLayer>
    concept IsLayer = std::is_base_of_v<Layer, TLayer>;
} // namespace sop