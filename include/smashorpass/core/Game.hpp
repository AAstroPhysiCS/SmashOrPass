#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Arena.hpp"
#include "smashorpass/core/InputBindings.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/SpriteAnimationPlayer.hpp"
#include "smashorpass/core/PlayerEffectEmitter.hpp"

#include "smashorpass/rendering/ParticleSystem.hpp"

namespace sop {

class Renderer;
struct AppCtx;

struct GameConfig {};

enum class GameMode : uint8_t { Smash, Deathmatch };

struct PlayerMatchState {
    std::string Name;
    int HP = 100;
    int Stocks = 3;
    int RoundsWon = 0;
    bool FacingRight = true;
};

struct PlayerState {
    PlayerInputState Input;
    PlayerCharacterState Character;
    PlayerMatchState Match;
    PlayerControlConfig Control;
    PlayerBindings Bindings;
};

inline static GameConfig loadDefault() {
    return GameConfig{};
}

class Game final {
   public:
    void OnEvent(AppCtx& ctx, const Event& event);
    void SetDisplayMetrics(const DisplayMetrics& metrics);
    void GameplayTick(double stepSeconds,
                      AssetManager& assetManager,
                      ParticleSystem& particleSystem);
    void AnimationTick(AssetManager& assetManager);
    void Render(Renderer& renderer,
                EventDispatcher& dispatcher,
                AssetManager& assetManager,
                bool renderCollisionBoxes);

   private:
    void EnsurePlayerCollisionProfile(AssetManager& assetManager);
    void UpdateArena(SDL_FPoint logicalSize);
    void AdvancePlayerAnimation(PlayerCharacterState& player, AssetManager& assetManager);
    void RenderWorld(Renderer& renderer,
                     EventDispatcher& dispatcher,
                     AssetManager& assetManager,
                     bool renderCollisionBoxes);
    void RenderStage(Renderer& renderer, AssetManager& assetManager);
    void RenderStageForeground(Renderer& renderer, AssetManager& assetManager);
    void RenderPlayers(Renderer& renderer, AssetManager& assetManager, EventDispatcher& dispatcher);
    void RenderCollisionBoxes(Renderer& renderer, AssetManager& assetManager);
    void RenderEffects(Renderer& renderer);

    void EmitPlayerParticleEffect(ParticleSystem& particleSystem,
                                  const PlayerParticleEffectEvent& event);
    void EmitSwordFireParticleEffect(ParticleSystem& particleSystem,
                                     const PlayerParticleEffectEvent& event);
    void EmitDashParticleEffect(ParticleSystem& particleSystem,
                                const PlayerParticleEffectEvent& event);

    PlayerState m_Player1{.Bindings{.MoveLeft = SDLK_A,
                                    .MoveRight = SDLK_D,
                                    .Jump = SDLK_W,
                                    .Dash = SDLK_LSHIFT,
                                    .Attack = SDLK_SPACE}};
    PlayerState m_Player2{.Bindings{.MoveLeft = SDLK_J,
                                    .MoveRight = SDLK_L,
                                    .Jump = SDLK_I,
                                    .Dash = SDLK_RSHIFT,
                                    .Attack = SDLK_M}};

    PlayerEffectEmitter m_PlayerEffectEmitter;

    ArenaId m_Arena = ArenaId::Chains;
    SDL_FRect m_ArenaRect{0.0f, 0.0f, kDefaultPlayerScreenWidth, kDefaultPlayerScreenHeight};
};
}  // namespace sop
