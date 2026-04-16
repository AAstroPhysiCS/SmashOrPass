#pragma once

#include <vector>
#include <memory>

#include "smashorpass/layer/Layer.hpp"

#include "smashorpass/platform/Window.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

    class Application {
    public:
        Application();
        ~Application();

        template <IsLayer TLayer, typename... TArgs>
        inline void PushLayer(TArgs&&... args) {
            m_Layers.push_back(std::make_unique<TLayer>(std::forward<TArgs>(args)...));
        }

        int Run();
    private:
        void ProcessEvents(bool& running);
        void Update();
        void Render();

        /* The order is important! */
        Window m_Window;
        Renderer m_Renderer;

        std::vector<std::unique_ptr<Layer>> m_Layers;
    };
}
