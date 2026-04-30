#pragma once

#include "smashorpass/core/ApplicationContext.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

struct Layer {
    Layer(Renderer& renderer, const Window& window, EventDispatcher& eventDispatcher)
        : m_Renderer(renderer), m_Window(window), m_EventDispatcher(eventDispatcher) {}
    virtual ~Layer() = default;

    virtual void OnEvent(const Event& event) = 0;
    virtual void OnGameplayTick(ApplicationContext&) {}
    virtual void OnAnimationTick(ApplicationContext&) {}
    virtual void OnUpdate(ApplicationContext& ctx) = 0;
    virtual void OnRender(ApplicationContext& ctx) = 0;

    [[nodiscard]] inline Renderer& GetRenderer() {
        return m_Renderer;
    }
    [[nodiscard]] inline const Window& GetWindow() const {
        return m_Window;
    }

   private:
    Renderer& m_Renderer;
    const Window& m_Window;
    EventDispatcher& m_EventDispatcher;
};

template <typename TLayer>
concept IsLayer = std::is_base_of_v<Layer, TLayer>;
}  // namespace sop
