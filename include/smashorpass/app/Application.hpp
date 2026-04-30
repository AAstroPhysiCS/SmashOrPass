#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "smashorpass/core/ApplicationContext.hpp"
#include "smashorpass/core/FixedStepScheduler.hpp"
#include "smashorpass/layer/Layer.hpp"
#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

class Application {
   public:
    Application();
    ~Application();

    template <typename TOverlay, typename... TArgs>
    inline void PushOverlay(TArgs&&... args) {
        m_Overlays.push_back(std::make_unique<TOverlay>(
            m_Renderer, m_Window, m_EventDispatcher, std::forward<TArgs>(args)...));
    }

    int Run();

   private:
    void ProcessEvents(bool& running);
    void TickGameplay(FixedStepScheduler::Duration elapsed);
    void TickAnimation(FixedStepScheduler::Duration elapsed);
    void Update();
    void Render();

    void DispatchEvent(const Event& event);
    void RefreshDisplayMetrics();
    void OnEvent(const Event& event);
    void OnApplicationStageChangeEvent();
    void ChangeState(ApplicationState newState);
    void ToggleDebugOverlay();

    /* The order is important! */
    Window m_Window;
    Renderer m_Renderer;
    EventDispatcher m_EventDispatcher;

    ApplicationContext m_Context{};
    FixedStepScheduler m_GameplayScheduler{120};
    FixedStepScheduler m_AnimationScheduler{60};

    std::vector<std::unique_ptr<Layer>> m_Overlays;
    std::unique_ptr<Layer> m_CurrentLayer;
    bool m_DebugOverlayVisible = false;
};
}  // namespace sop
