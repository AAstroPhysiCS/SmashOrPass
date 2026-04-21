#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "ApplicationState.hpp"

namespace sop {

    class AssetManager;
    class Renderer;

    struct GameConfig {

    };
    
    enum class GameMode : uint8_t { 
        Smash, 
        Deathmatch 
    };

    struct PlayerMatchState {
        std::string Name;
        int HP = 100;
        int Stocks = 3;
        int RoundsWon = 0;
        bool FacingRight = true;
    };

    inline static GameConfig loadDefault() {
        return GameConfig{}; 
    }

    class Game final {
    public:
        void OnEvent(const Event& event);
        void Update(ApplicationState state);
        void Render(ApplicationState state, Renderer& renderer, AssetManager& assetManager);
    private:
        void RenderWorld(Renderer& renderer, AssetManager& assetManager);
        void RenderStage(Renderer& renderer);
        void RenderPlayers(Renderer& renderer, AssetManager& assetManager);
        void RenderEffects(Renderer& renderer);
    };
}
