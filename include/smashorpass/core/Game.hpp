#pragma once

#include <cstdint>
#include <string>

#include "ApplicationState.hpp"
#include "Event.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Arena.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/SpriteAnimationPlayer.hpp"
#include "smashorpass/rendering/ParticleSystem.hpp"

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
    void SetDisplayMetrics(const DisplayMetrics& metrics);
    void GameplayTick(ApplicationState state,
                      double stepSeconds,
                      AssetManager& assetManager,
                      ParticleSystem& particleSystem);
    void AnimationTick(ApplicationState state, AssetManager& assetManager);
    void Render(ApplicationState state,
                Renderer& renderer,
                AssetManager& assetManager,
                bool renderCollisionBoxes);

   private:
    void EnsurePlayerCollisionProfile(AssetManager& assetManager);
    void UpdateArena(SDL_FPoint logicalSize);
    void AdvancePlayerAnimation(PlayerCharacterState& player, AssetManager& assetManager);
    void RenderWorld(Renderer& renderer, AssetManager& assetManager, bool renderCollisionBoxes);
    void RenderStage(Renderer& renderer, AssetManager& assetManager);
    void RenderStageForeground(Renderer& renderer, AssetManager& assetManager);
    void RenderPlayers(Renderer& renderer, AssetManager& assetManager);
    void RenderCollisionBoxes(Renderer& renderer, AssetManager& assetManager);
    void RenderEffects(Renderer& renderer);

    PlayerControlConfig m_PlayerConfig;
    PlayerInputState m_PlayerInput;
    PlayerCharacterState m_Player;
    ArenaId m_Arena = ArenaId::Chains;
    SDL_FRect m_ArenaRect{0.0f, 0.0f, kDefaultPlayerScreenWidth, kDefaultPlayerScreenHeight};
};
}  // namespace sop
