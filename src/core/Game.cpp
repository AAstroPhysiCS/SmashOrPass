#include "smashorpass/core/Game.hpp"

#include <SDL3/SDL_keycode.h>

#include <span>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/PlayerSpritePlacement.hpp"
#include "smashorpass/rendering/Renderer.hpp"
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

void Game::OnEvent(const Event& event) {
    EventDispatcher::Dispatch<KeyEvent>(event, [this](const KeyEvent& keyEvent) {
        ApplyBindings(m_Player1.Input, keyEvent, m_Player1.Bindings);
        ApplyBindings(m_Player2.Input, keyEvent, m_Player2.Bindings);
    });

    EventDispatcher::Dispatch<WindowMetricsChangedEvent>(
        event, [this](const WindowMetricsChangedEvent& e) { SetDisplayMetrics(e.Metrics); });
}

void Game::SetDisplayMetrics(const DisplayMetrics& metrics) {
    UpdateArena(metrics.LogicalSize());
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
                  AssetManager& assetManager,
                  bool renderCollisionBoxes) {
    if (state == ApplicationState::Playing) {
        RenderWorld(renderer, assetManager, renderCollisionBoxes);
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

void Game::RenderWorld(Renderer& renderer, AssetManager& assetManager, bool renderCollisionBoxes) {
    EnsurePlayerCollisionProfile(assetManager);
    UpdateArena(renderer.GetLogicalOutputSize());
    RenderStage(renderer, assetManager);
    RenderPlayers(renderer, assetManager);
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

void Game::RenderPlayers(Renderer& renderer, AssetManager& assetManager) {
    const auto DrawPlayer = [&](const PlayerCharacterState& player,
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

        TextureDrawParams drawParams{};
        drawParams.src = &placement.SourceRect;
        drawParams.dst = placement.DestinationRect;
        drawParams.origin = placement.Origin;
        drawParams.flip = placement.Flip;

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
}  // namespace sop
