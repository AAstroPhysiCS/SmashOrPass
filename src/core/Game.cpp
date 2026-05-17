#include "smashorpass/core/Game.hpp"
#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL_keycode.h>

#include <span>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/app/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/PlayerSpritePlacement.hpp"
#include "spdlog/spdlog.h"

namespace sop {
namespace {

[[nodiscard]] SDL_FPoint MapDesignPointToArena(SDL_FPoint designPoint, const SDL_FRect& arenaRect) {
    if (arenaRect.w <= 0.0f || arenaRect.h <= 0.0f) {
        return SDL_FPoint{};
    }

    return SDL_FPoint{
        arenaRect.x + (designPoint.x * (arenaRect.w / kDefaultArenaWidth)),
        arenaRect.y + (designPoint.y * (arenaRect.h / kDefaultArenaHeight)),
    };
}

[[nodiscard]] float MapDesignScaleToArena(const SDL_FRect& arenaRect) {
    if (arenaRect.h <= 0.0f) {
        return 0.0f;
    }

    return arenaRect.h / kDefaultArenaHeight;
}

}  // namespace

void Game::OnEvent(AppCtx& ctx, const Event& event) {
    EventDispatcher::Dispatch<KeyEvent>(event, [this](const KeyEvent& keyEvent) {
        ApplyBindings(m_Player1.Input, keyEvent, m_Player1.Bindings);
        ApplyBindings(m_Player2.Input, keyEvent, m_Player2.Bindings);
    });

    EventDispatcher::Dispatch<WindowMetricsChangedEvent>(
        event, [this](const WindowMetricsChangedEvent& e) { SetDisplayMetrics(e.Metrics); });

    EventDispatcher::Dispatch<PlayerParticleEffectEvent>(
        event, [&](const PlayerParticleEffectEvent& particleEvent) {
            EmitPlayerParticleEffect(ctx.m_ParticleSystem, particleEvent);
        });
}

void Game::SetDisplayMetrics(const DisplayMetrics& metrics) {
    UpdateArena(metrics.LogicalSize());
}

void Game::GameplayTick(double stepSeconds,
                        AssetManager& assetManager,
                        ParticleSystem& particleSystem) {
    EnsurePlayerCollisionProfile(assetManager);
    ApplyPlayerViewport(m_Player1.Control, m_Player1.Character, m_ArenaRect);
    ApplyPlayerViewport(m_Player2.Control, m_Player2.Character, m_ArenaRect);
    std::span<const SDL_FRect> arenaCollisions = assetManager.getArenaCollisionBoxes(m_Arena);
    TickPlayer(m_Player1.Character,
               m_Player1.Input,
               stepSeconds,
               m_Player1.Control,
               arenaCollisions,
               particleSystem);
    TickPlayer(m_Player2.Character,
               m_Player2.Input,
               stepSeconds,
               m_Player2.Control,
               arenaCollisions,
               particleSystem);
}

void Game::AnimationTick(AssetManager& assetManager) {
    AdvancePlayerAnimation(m_Player1.Character, assetManager);
    AdvancePlayerAnimation(m_Player2.Character, assetManager);
}

void Game::Render(Renderer& renderer,
                  EventDispatcher& dispatcher,
                  AssetManager& assetManager,
                  bool renderCollisionBoxes) {
    RenderWorld(renderer, dispatcher, assetManager, renderCollisionBoxes);
}

void Game::EnsurePlayerCollisionProfile(AssetManager& assetManager) {
    if (m_Player1.Character.CollisionProfileInitialized &&
        m_Player2.Character.CollisionProfileInitialized) {
        return;
    }

    const SpriteSheet& spriteSheet1 =
        assetManager.getSpriteSheet(m_Player1.Character.Character, CharacterAnimation::Idle);
    const std::span<const SpriteSheetFrame> frames1 = spriteSheet1.getFrames();
    SOP_ASSERT(!frames1.empty(), "Character idle sprite sheet has no frames");

    const SpriteSheet& spriteSheet2 =
        assetManager.getSpriteSheet(m_Player2.Character.Character, CharacterAnimation::Idle);
    const std::span<const SpriteSheetFrame> frames2 = spriteSheet2.getFrames();
    SOP_ASSERT(!frames2.empty(), "Character idle sprite sheet has no frames");

    ApplyPlayerCollisionProfile(m_Player1.Character, frames1.front(), m_Player1.Control);
    ApplyPlayerCollisionProfile(m_Player2.Character, frames2.front(), m_Player2.Control);
    SetPlayerSpawn(m_Player1.Character, 450.0f, 448.0f, true);
    SetPlayerSpawn(m_Player2.Character, 1354.0f, 448.0f, false);
    ApplyPlayerViewport(m_Player1.Control, m_Player1.Character, m_ArenaRect);
    ApplyPlayerViewport(m_Player2.Control, m_Player2.Character, m_ArenaRect);
}

void Game::AdvancePlayerAnimation(PlayerCharacterState& player, AssetManager& assetManager) {
    const SpriteSheet& spriteSheet =
        assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
    const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
    SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

    player.Animation.Advance(frames.size());
}

void Game::RenderWorld(Renderer& renderer, EventDispatcher& dispatcher, AssetManager& assetManager, bool renderCollisionBoxes) {
    EnsurePlayerCollisionProfile(assetManager);
    UpdateArena(renderer.GetLogicalOutputSize());
    RenderStage(renderer, assetManager);
    RenderPlayers(renderer, assetManager, dispatcher);
    RenderEffects(renderer);
    RenderStageForeground(renderer, assetManager);
    if (renderCollisionBoxes) {
        RenderCollisionBoxes(renderer, assetManager);
    }
}

void Game::UpdateArena(SDL_FPoint logicalSize) {
    m_ArenaRect = MakeContainedArenaRect(logicalSize);
    ApplyPlayerViewport(m_Player1.Control, m_Player1.Character, m_ArenaRect);
    ApplyPlayerViewport(m_Player2.Control, m_Player2.Character, m_ArenaRect);
}

void Game::RenderStage(Renderer& renderer, AssetManager& assetManager) {
    const SDL_FPoint size = renderer.GetLogicalOutputSize();

    renderer.FillRect(SDL_FRect{0.0f, 0.0f, size.x, size.y}, Color{18, 18, 24, 255});

    const bool arenaDrawn =
        renderer.DrawTexture(assetManager.getArenaBackgroundTexture(m_Arena), m_ArenaRect);
    SOP_VERIFY(arenaDrawn, "Failed to draw arena background");
}

void Game::RenderStageForeground(Renderer& renderer, AssetManager& assetManager) {
    const bool arenaDrawn =
        renderer.DrawTexture(assetManager.getArenaForegroundTexture(m_Arena), m_ArenaRect);
    SOP_VERIFY(arenaDrawn, "Failed to draw arena foreground");
}

void Game::RenderPlayers(Renderer& renderer, AssetManager& assetManager, EventDispatcher& dispatcher) {
    const auto DrawPlayer = [&](PlayerCharacterState& player,
                                const PlayerControlConfig& control) {
        const SpriteSheet& spriteSheet =
            assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
        const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
        SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

        const SpriteSheetFrame& frame = frames[player.Animation.GetFrameIndex() % frames.size()];
        const SDL_FPoint anchorPosition = MapDesignPointToArena(player.AnchorPosition, m_ArenaRect);
        const float scale = control.RenderScale * MapDesignScaleToArena(m_ArenaRect);
        const detail::PlayerSpritePlacement placement =
            detail::MakePlayerSpritePlacement(anchorPosition, frame, player.FacingRight, scale);

        player.Position = Vec2{anchorPosition.x, anchorPosition.y};

        TextureDrawParams drawParams{};
        drawParams.src = &placement.SourceRect;
        drawParams.dst = placement.DestinationRect;
        drawParams.origin = placement.Origin;
        drawParams.flip = placement.Flip;

        m_PlayerEffectEmitter
            .Update(player, player.Animation.GetAnimation(), player.Animation.GetFrameIndex(), frame,
                    assetManager.GetCharacterAnimationEffectMasks(player.Character, player.Animation.GetAnimation(), EffectMaskKind::SwordGreen), scale, 0.0, dispatcher);

        return renderer.DrawTexture(spriteSheet.getSpriteTexture(), drawParams);
    };

    const bool playerDrawn1 = DrawPlayer(m_Player1.Character, m_Player1.Control);
    SOP_VERIFY(playerDrawn1, "Failed to draw player 1 sprite");
    const bool playerDrawn2 = DrawPlayer(m_Player2.Character, m_Player2.Control);
    SOP_VERIFY(playerDrawn2, "Failed to draw player 2 sprite");
}

void Game::RenderCollisionBoxes(Renderer& renderer, AssetManager& assetManager) {
    constexpr Color kArenaCollisionBoxColor{0, 255, 0, 255};
    constexpr Color kPlayerCollisionBoxColor{255, 230, 0, 255};

    for (const SDL_FRect& designRect : assetManager.getArenaCollisionBoxes(m_Arena)) {
        const SDL_FRect arenaRect = MapDesignRectToArena(designRect, m_ArenaRect);
        const bool boxDrawn = renderer.DrawRect(arenaRect, kArenaCollisionBoxColor);
        SOP_VERIFY(boxDrawn, "Failed to draw arena collision box");
    }

    const SDL_FRect playerRect1 =
        MapDesignRectToArena(m_Player1.Character.CollisionRect, m_ArenaRect);
    const SDL_FRect playerRect2 =
        MapDesignRectToArena(m_Player2.Character.CollisionRect, m_ArenaRect);
    const bool playerBoxDrawn1 = renderer.DrawRect(playerRect1, kPlayerCollisionBoxColor);
    SOP_VERIFY(playerBoxDrawn1, "Failed to draw player 1 collision box");
    const bool playerBoxDrawn2 = renderer.DrawRect(playerRect2, kPlayerCollisionBoxColor);
    SOP_VERIFY(playerBoxDrawn2, "Failed to draw player 2 collision box");
}

void Game::RenderEffects(Renderer&) {
    // TODO:
    // attack trails
    // hit sparks
    // dust
    // particles
}
void Game::EmitPlayerParticleEffect(ParticleSystem& particleSystem,
                                    const PlayerParticleEffectEvent& event) {
    switch (event.Type) {
        case PlayerParticleEffectType::SwordFire:
            EmitSwordFireParticleEffect(particleSystem, event);
            break;

        case PlayerParticleEffectType::DashBlue:
            EmitDashParticleEffect(particleSystem, event);
            break;
    }
}
void Game::EmitSwordFireParticleEffect(ParticleSystem& particleSystem,
                                       const PlayerParticleEffectEvent& event) {
    ParticleBurstDesc desc{};
    desc.Position = Vec2{event.Position.x, event.Position.y};
    desc.InitialVelocity = Vec2{
        event.FacingRight ? -20.0f : 20.0f,
        -40.0f,
    };

    desc.Count = 3;
    desc.MinSpeed = 20.0f;
    desc.MaxSpeed = 110.0f;
    desc.MinLifetime = 0.12f;
    desc.MaxLifetime = 0.28f;
    desc.MinSize = 8.0f;
    desc.MaxSize = 18.0f;

    desc.StartColor = Color{255, 150, 40, 180};
    desc.EndColor = Color{255, 25, 0, 0};

    desc.Acceleration = Vec2{
        event.FacingRight ? -40.0f : 40.0f,
        -220.0f,
    };

    particleSystem.EmitBurst(desc);
}
void Game::EmitDashParticleEffect(ParticleSystem& particleSystem,
                                  const PlayerParticleEffectEvent& event) {
    ParticleBurstDesc desc{};
    desc.Position = Vec2{event.Position.x, event.Position.y};

    desc.InitialVelocity = Vec2{
        event.Velocity.x,
        event.Velocity.y,
    };

    desc.Count = 6;
    desc.MinSpeed = 40.0f;
    desc.MaxSpeed = 180.0f;
    desc.MinLifetime = 0.16f;
    desc.MaxLifetime = 0.34f;
    desc.MinSize = 10.0f;
    desc.MaxSize = 24.0f;

    desc.StartColor = Color{80, 180, 255, 150};
    desc.EndColor = Color{40, 80, 255, 0};

    desc.Acceleration = Vec2{0.0f, 0.0f};

    particleSystem.EmitBurst(desc);
}
}  // namespace sop
