#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/state/states/in_game/Arena.hpp"
#include "smashorpass/state/states/in_game/InputBindings.hpp"
#include "smashorpass/state/states/in_game/PlayerController.hpp"
#include "smashorpass/state/states/in_game/PlayerEffectEmitter.hpp"
#include "smashorpass/state/states/in_game/SpriteAnimationPlayer.hpp"

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
    void GameplayTick(AppCtx& ctx, std::chrono::steady_clock::duration step);
    void AnimationTick(AppCtx& ctx);
    void Render(AppCtx& ctx);

   private:
    void EnsurePlayerCollisionProfile(AppCtx& ctx);
    void UpdateArena(SDL_FPoint logicalSize);
    void AdvancePlayerAnimation(AppCtx& ctx, PlayerCharacterState& player);
    void RenderWorld(AppCtx& ctx);
    void RenderStage(AppCtx& ctx);
    void RenderStageForeground(AppCtx& ctx);
    void RenderPlayers(AppCtx& ctx);
    void RenderCollisionBoxes(AppCtx& ctx);
    void RenderEffects(AppCtx& ctx);

    void EmitPlayerParticleEffect(AppCtx& ctx, const PlayerParticleEffectEvent& event);
    void EmitSwordFireParticleEffect(AppCtx& ctx, const PlayerParticleEffectEvent& event);
    void EmitDashParticleEffect(AppCtx& ctx, const PlayerParticleEffectEvent& event);

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
