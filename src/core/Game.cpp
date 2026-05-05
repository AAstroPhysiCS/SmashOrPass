#include "smashorpass/core/Game.hpp"

#include <SDL3/SDL_keycode.h>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/core/PlayerController.hpp"
#include "smashorpass/core/PlayerSpritePlacement.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "spdlog/spdlog.h"

namespace sop {

void Game::OnEvent(const Event& event) {
    EventDispatcher::Dispatch<KeyEvent>(event, [this](const KeyEvent& keyEvent) {
        ApplyPlayerKeyEvent(m_PlayerInput, m_Player, keyEvent, m_PlayerConfig);
    });

    EventDispatcher::Dispatch<WindowMetricsChangedEvent>(
        event, [this](const WindowMetricsChangedEvent& e) { SetDisplayMetrics(e.Metrics); });
}

void Game::SetDisplayMetrics(const DisplayMetrics& metrics) {
    UpdateArena(metrics.LogicalSize());
}

void Game::GameplayTick(ApplicationState state, double stepSeconds, AssetManager& assetManager) {
    switch (state) {
        case ApplicationState::MainMenu:
            // spdlog::info("In main menu");
            //  TODO: main menu, handle menu input, etc.
            break;
        case ApplicationState::CharacterSelect:
            // spdlog::info("In character select");
            //  TODO: character select, handle character select input, etc.
            break;
        case ApplicationState::Playing:
            // spdlog::info("Playing");
            TickPlayer(m_Player,
                       m_PlayerInput,
                       stepSeconds,
                       m_PlayerConfig,
                       assetManager.getArenaCollisionBoxes(m_Arena));
            break;
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

    AdvancePlayerAnimation(m_Player, assetManager);
}

void Game::Render(ApplicationState state,
                  Renderer& renderer,
                  AssetManager& assetManager,
                  bool renderCollisionBoxes) {
    if (state == ApplicationState::Playing) {
        RenderWorld(renderer, assetManager, renderCollisionBoxes);
    }
}

void Game::AdvancePlayerAnimation(PlayerCharacterState& player, AssetManager& assetManager) {
    const SpriteSheet& spriteSheet =
        assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
    const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
    SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

    player.Animation.Advance(frames.size());
}

void Game::RenderWorld(Renderer& renderer,
                       AssetManager& assetManager,
                       bool renderCollisionBoxes) {
    UpdateArena(renderer.GetLogicalOutputSize());
    RenderStage(renderer, assetManager);
    RenderPlayers(renderer, assetManager);
    if (renderCollisionBoxes) {
        RenderArenaCollisionBoxes(renderer, assetManager);
    }
    RenderEffects(renderer);
}

void Game::UpdateArena(SDL_FPoint logicalSize) {
    m_ArenaRect = MakeContainedArenaRect(logicalSize);
    ApplyPlayerViewport(m_PlayerConfig, m_Player, m_ArenaRect);
}

void Game::RenderStage(Renderer& renderer, AssetManager& assetManager) {
    const SDL_FPoint size = renderer.GetLogicalOutputSize();

    renderer.FillRect(SDL_FRect{0.0f, 0.0f, size.x, size.y}, Color{18, 18, 24, 255});

    const bool arenaDrawn =
        renderer.DrawTexture(assetManager.getArenaTexture(m_Arena), m_ArenaRect);
    SOP_VERIFY(arenaDrawn, "Failed to draw arena background");
}

void Game::RenderPlayers(Renderer& renderer, AssetManager& assetManager) {
    const auto DrawPlayer = [&](const PlayerCharacterState& player,
                                const SDL_FRect& designPlaceholderRect) {
        const SpriteSheet& spriteSheet =
            assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
        const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
        SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

        const SpriteSheetFrame& frame = frames[player.Animation.GetFrameIndex() % frames.size()];
        const SDL_FRect placeholderRect = MapDesignRectToArena(designPlaceholderRect, m_ArenaRect);
        const detail::PlayerSpritePlacement placement = detail::MakePlayerSpritePlacement(
            placeholderRect, frame, player.FacingRight, m_PlayerConfig.RenderReferenceSourceHeight);

        TextureDrawParams drawParams{};
        drawParams.src = &placement.SourceRect;
        drawParams.dst = placement.DestinationRect;
        drawParams.origin = placement.Origin;
        drawParams.flip = placement.Flip;

        return renderer.DrawTexture(spriteSheet.getSpriteTexture(), drawParams);
    };

    const bool playerDrawn = DrawPlayer(m_Player, m_Player.PlaceholderRect);
    SOP_VERIFY(playerDrawn, "Failed to draw player sprite");
}

void Game::RenderArenaCollisionBoxes(Renderer& renderer, AssetManager& assetManager) {
    constexpr Color kCollisionBoxColor{0, 255, 0, 255};

    for (const SDL_FRect& designRect : assetManager.getArenaCollisionBoxes(m_Arena)) {
        const SDL_FRect arenaRect = MapDesignRectToArena(designRect, m_ArenaRect);
        const bool boxDrawn = renderer.DrawRect(arenaRect, kCollisionBoxColor);
        SOP_VERIFY(boxDrawn, "Failed to draw arena collision box");
    }
}

void Game::RenderEffects(Renderer&) {
    // TODO:
    // attack trails
    // hit sparks
    // dust
    // particles
}
}  // namespace sop
