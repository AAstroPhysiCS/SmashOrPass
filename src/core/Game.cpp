#include "smashorpass/core/Game.hpp"
#include "smashorpass/core/Base.hpp"

#include <SDL3/SDL_keycode.h>

#include <span>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/PlayerSpritePlacement.hpp"
#include "smashorpass/core/ApplicationContext.hpp"
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

void Game::OnEvent(const Event& event, ApplicationContext& ctx) {
    EventDispatcher::Dispatch<KeyEvent>(event, [this](const KeyEvent& keyEvent) {
        ApplyBindings(m_Player1.Input, keyEvent, m_Player1.Bindings);
        ApplyBindings(m_Player2.Input, keyEvent, m_Player2.Bindings);
    });

    EventDispatcher::Dispatch<WindowMetricsChangedEvent>(
        event, [this](const WindowMetricsChangedEvent& e) { SetDisplayMetrics(e.Metrics); });

    EventDispatcher::Dispatch<PlayerParticleEffectEvent>(
        event, [&](const PlayerParticleEffectEvent& particleEvent) {
            EmitPlayerParticleEffect(ctx.Particles, particleEvent);
        });
}

void Game::SetDisplayMetrics(const DisplayMetrics& metrics) {
    UpdateArena(metrics.LogicalSize());
}

Vec2 getOverlap(SDL_FRect rect1, SDL_FRect rect2) {
    const float center1X = rect1.x + rect1.w * 0.5f;
    const float center1Y = rect1.y + rect1.h * 0.5f;
    const float center2X = rect2.x + rect2.w * 0.5f;
    const float center2Y = rect2.y + rect2.h * 0.5f;

    const float deltaX = center2X - center1X;
    const float deltaY = center2Y - center1Y;

    const float overlapX = (rect1.w + rect2.w) * 0.5f - std::abs(deltaX);
    const float overlapY = (rect1.h + rect2.h) * 0.5f - std::abs(deltaY);

    if (overlapX <= 0.0f || overlapY <= 0.0f) {
        return Vec2{0.0f, 0.0f};
    }

    return Vec2{
        deltaX >= 0.0f ? overlapX : -overlapX,
        deltaY >= 0.0f ? overlapY : -overlapY,
    };
}

void GroundPlayer(PlayerCharacterState& player) {
    player.VerticalVelocity = 0.0f;
    player.Grounded = true;
    player.AirDashAvailable = true;
    player.AirJumpAvailable = false;
}

void HitCeiling(PlayerCharacterState& player) {
    player.VerticalVelocity = 0.0f;
}

void pushPlayer(PlayerCharacterState& player, Vec2 distance, float factor, bool pushY) {
    const float dx = distance.x * factor;
    const float dy = distance.y * factor;
    if (pushY) {
        player.AnchorPosition.y += dy;
        player.CollisionBox.Rect.y += dy;
        if (dy < -0.0001f) {// < -0.0001f prevents a "lag" during fall, when this player stands on top of a jumping player
            GroundPlayer(player);
        } else if (dy > 0.0f) {
            HitCeiling(player);
        }
    } else {
        player.AnchorPosition.x += dx;
        player.CollisionBox.Rect.x += dx;
    }
}

void pushBoxes(PlayerCharacterState& player, SDL_FRect platformRect) {
    Vec2 overlap = getOverlap(player.CollisionBox.Rect, platformRect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return;
    }
    if (overlap.y > 0.00001f && player.CollisionBox.Push.canPushUp
        && overlap.y < (player.CollisionBox.Rect.h * 0.1f) && player.VerticalVelocity >= 0.0f) {
        // p1 is above -> move p1 up
        pushPlayer(player, overlap, -1.0f, true);
    } else if (overlap.y < -0.00001f
        && overlap.y > (player.CollisionBox.Rect.h * -0.1f) && player.VerticalVelocity <= 0.0f) {
        // platform is above -> move player down
        pushPlayer(player, overlap, -1.0f, true);
    } else {
        // try to move on x
        const bool playerNeedsToMoveLeft = overlap.x > 0.0f;
        const bool playerCanMoveX = playerNeedsToMoveLeft ? player.CollisionBox.Push.canPushLeft : player.CollisionBox.Push.canPushRight;
        if (!playerCanMoveX) {
            return;
        }

        if (playerCanMoveX) {
            pushPlayer(player, overlap, -1.0f, false);
            if (overlap.x > 0.0f) {
                player.CollisionBox.Push.canPushRight = false;
            } else {
                player.CollisionBox.Push.canPushLeft = false;
            }
        }
    }
}

void pushBoxes(PlayerCharacterState& player1, PlayerCharacterState& player2) {
    Vec2 overlap = getOverlap(player1.CollisionBox.Rect, player2.CollisionBox.Rect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return;
    }
    /*
        If there is a slight height overlap (10% of collision box) and the player 
        comes from above, then he is simply moved onto the other player.
        Otherwise move the players on the x-axis.
    */ 
    if (overlap.y > 0.00001f && player1.CollisionBox.Push.canPushUp
        && overlap.y < (player1.CollisionBox.Rect.h * 0.1f) && player1.VerticalVelocity >= 0.0f) {
        // p1 is above -> move p1 up
        pushPlayer(player1, overlap, -1.0f, true);
    } else if (overlap.y < -0.00001f && player2.CollisionBox.Push.canPushUp
        && overlap.y > (player2.CollisionBox.Rect.h * -0.1f) && player2.VerticalVelocity >= 0.0f) {
        // p2 is above -> move p2 up
        pushPlayer(player2, overlap, 1.0f, true);
    } else {
        // try to move on x
        const bool player1NeedsToMoveLeft = overlap.x > 0.0f;
        const bool player1CanMoveX = player1NeedsToMoveLeft ? player1.CollisionBox.Push.canPushLeft : player1.CollisionBox.Push.canPushRight;
        const bool player2CanMoveX = player1NeedsToMoveLeft ? player2.CollisionBox.Push.canPushRight : player2.CollisionBox.Push.canPushLeft;
        if (!player1CanMoveX && !player2CanMoveX) {
            return;
        }

        if (player1CanMoveX && player2CanMoveX) {
            const float totalWeight = player1.CollisionBox.Weight + player2.CollisionBox.Weight;
            if (totalWeight <= 0.0f) {
                pushPlayer(player1, overlap, -0.5f, false);
                pushPlayer(player2, overlap, 0.5f, false);
            } else {
                const float player1PushFactor = -player2.CollisionBox.Weight / totalWeight;
                const float player2PushFactor = player1.CollisionBox.Weight / totalWeight;
                pushPlayer(player1, overlap, player1PushFactor, false);
                pushPlayer(player2, overlap, player2PushFactor, false);
            }
        } else if (player1CanMoveX) {
            pushPlayer(player1, overlap, -1.0f, false);
        } else {
            pushPlayer(player2, overlap, 1.0f, false);
        }
    }
}

void SolveCollisions(PlayerState& player1, PlayerState& player2, std::span<const SDL_FRect> arenaCollisions) {
    // TODO: platform collisions
    for (const SDL_FRect& platform : arenaCollisions) {
        pushBoxes(player1.Character, platform);
        pushBoxes(player2.Character, platform);
    }
    pushBoxes(player1.Character, player2.Character);
    player1.Character.CollisionBox.Push = {true, true, true, false};
    player2.Character.CollisionBox.Push = {true, true, true, false};
}

void Game::GameplayTick(ApplicationState state,
                        double stepSeconds,
                        AssetManager& assetManager,
                        ParticleSystem& particleSystem) {
    switch (state) {
        case ApplicationState::MainMenu:
            // spdlog::info("In main menu");
            //  TODO: main menu, handle menu input, etc.
            break;
        case ApplicationState::CharacterSelect:
            // spdlog::info("In character select");
            //  TODO: character select, handle character select input, etc.
            break;
        case ApplicationState::Playing: {
            // spdlog::info("Playing");
            EnsurePlayerCollisionProfile(assetManager);
            ApplyPlayerViewport(m_Player1.Control, m_Player1.Character, m_ArenaRect);
            ApplyPlayerViewport(m_Player2.Control, m_Player2.Character, m_ArenaRect);
            std::span<const SDL_FRect> arenaCollisions =
                assetManager.getArenaCollisionBoxes(m_Arena);
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
            SolveCollisions(m_Player1, m_Player2, arenaCollisions);
            break;
        }
        case ApplicationState::Paused:
            // spdlog::info("Paused");
            //  TODO: pause menu, handle pause menu input, etc.
            break;
        case ApplicationState::GameOver:
            // spdlog::info("Game over");
            //  TODO: game over screen, handle game over screen input, etc.
            break;
    }
    
}

void Game::AnimationTick(ApplicationState state, AssetManager& assetManager) {
    if (state != ApplicationState::Playing) {
        return;
    }

    AdvancePlayerAnimation(m_Player1.Character, assetManager);
    AdvancePlayerAnimation(m_Player2.Character, assetManager);
}

void Game::Render(ApplicationState state,
                  Renderer& renderer,
                  EventDispatcher& dispatcher,
                  AssetManager& assetManager,
                  bool renderCollisionBoxes) {
    if (state == ApplicationState::Playing) {
        RenderWorld(renderer, dispatcher, assetManager, renderCollisionBoxes);
    }
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
        MapDesignRectToArena(m_Player1.Character.CollisionBox.Rect, m_ArenaRect);
    const SDL_FRect playerRect2 =
        MapDesignRectToArena(m_Player2.Character.CollisionBox.Rect, m_ArenaRect);
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
