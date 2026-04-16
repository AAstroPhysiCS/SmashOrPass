#pragma once

#include "smashorpass/core/Game.hpp"

#include "smashorpass/layer/Layer.hpp"

namespace sop {
    class GameLayer : public Layer {
    public:
        virtual ~GameLayer() {}

        void OnEvent(const Event& event) final override;
        void OnUpdate() final override;
        void OnRender() final override;
    private:
        Game m_Game;
    };
}  // namespace sop