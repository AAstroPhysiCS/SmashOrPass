#pragma once

#include <vector>
#include <memory>

#include "smashorpass/layer/Layer.hpp"

#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/core/ApplicationContext.hpp"

namespace sop {

    class Application {
    public:
        Application();
        ~Application();

        template <typename TOverlay, typename... TArgs>
        inline void PushOverlay(TArgs&&... args) {
            //m_Overlays.push_back(std::make_unique<TOverlay>(
             //   m_Renderer, m_Window, m_EventDispatcher, std::forward<TArgs>(args)...));
        }

        int Run();
    private:
        void ProcessEvents(bool& running);
        void Update();
        void Render();

        void OnEvent(const Event& event);
        void OnApplicationStageChangeEvent();
        void ChangeState(ApplicationState newState);

        /* The order is important! */
        Window m_Window;
        Renderer m_Renderer;
        EventDispatcher m_EventDispatcher;

        ApplicationContext m_Context{};

        //std::vector<std::unique_ptr<Overlay>> m_Overlays;
        std::unique_ptr<Layer> m_CurrentLayer;
    };
}
