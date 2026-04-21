#pragma once

#include <cstdint>
#include <string>

#include "ApplicationState.hpp"
#include "Event.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/SpriteAnimationPlayer.hpp"

namespace sop {

class Renderer;

struct GameConfig {};

enum class GameMode : uint8_t { Smash, Deathmatch };

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
    void GameplayTick(ApplicationState state);
    void AnimationTick(ApplicationState state, AssetManager& assetManager);
    void Render(ApplicationState state, Renderer& renderer, AssetManager& assetManager);

   private:
    struct CharacterVisualState {
        CharacterId Character = CharacterId::Robot;
        SpriteAnimationPlayer Animation;
        bool FacingRight = true;
    };

    void AdvancePlayerAnimation(CharacterVisualState& player, AssetManager& assetManager);
    void RenderWorld(Renderer& renderer, AssetManager& assetManager);
    void RenderStage(Renderer& renderer);
    void RenderPlayers(Renderer& renderer, AssetManager& assetManager);
    void RenderEffects(Renderer& renderer);

    CharacterVisualState m_Player1Visual{
        CharacterId::Robot, SpriteAnimationPlayer{CharacterAnimation::Attacks}, true};
    CharacterVisualState m_Player2Visual{
        CharacterId::Robot, SpriteAnimationPlayer{CharacterAnimation::Attacks}, false};
};
}  // namespace sop
