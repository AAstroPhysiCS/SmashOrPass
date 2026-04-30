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

void Game::GameplayTick(ApplicationState state, double stepSeconds) {
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
            TickPlayer(m_Player, m_PlayerInput, stepSeconds, m_PlayerConfig);
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

void Game::Render(ApplicationState state, Renderer& renderer, AssetManager& assetManager) {
    if (state == ApplicationState::Playing) {
        RenderWorld(renderer, assetManager);
    }
}

void Game::AdvancePlayerAnimation(PlayerCharacterState& player, AssetManager& assetManager) {
    const SpriteSheet& spriteSheet =
        assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
    const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
    SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

    player.Animation.Advance(frames.size());
}

void Game::RenderWorld(Renderer& renderer, AssetManager& assetManager) {
    UpdateArena(renderer.GetLogicalOutputSize());
    RenderStage(renderer);
    RenderPlayers(renderer, assetManager);
    RenderEffects(renderer);
}

void Game::UpdateArena(SDL_FPoint logicalSize) {
    m_ArenaRect = MakeContainedArenaRect(logicalSize);
    ApplyPlayerViewport(m_PlayerConfig, m_Player, m_ArenaRect);
}

void Game::RenderStage(Renderer& renderer) {
    const SDL_FPoint size = renderer.GetLogicalOutputSize();

    renderer.FillRect(SDL_FRect{0.0f, 0.0f, size.x, size.y}, Color{18, 18, 24, 255});

    renderer.FillRect(m_ArenaRect, Color{32, 36, 52, 255});
    renderer.DrawRect(m_ArenaRect, Color{180, 180, 190, 255});

    const float floorY = m_PlayerConfig.GroundY + m_Player.PlaceholderRect.h;

    renderer.DrawLine(
        m_ArenaRect.x, floorY, m_ArenaRect.x + m_ArenaRect.w, floorY, Color{180, 180, 190, 255});
}

void Game::RenderPlayers(Renderer& renderer, AssetManager& assetManager) {
    const auto DrawPlayer = [&](const PlayerCharacterState& player,
                                const SDL_FRect& placeholderRect) {
        const SpriteSheet& spriteSheet =
            assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
        const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
        SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

        const SpriteSheetFrame& frame = frames[player.Animation.GetFrameIndex() % frames.size()];
        const detail::PlayerSpritePlacement placement = detail::MakePlayerSpritePlacement(
            placeholderRect, frame, player.FacingRight, m_PlayerConfig.RenderReferenceSourceHeight);

        TextureDrawParams drawParams{};
        drawParams.src = &placement.SourceRect;
        drawParams.dst = placement.DestinationRect;
        drawParams.origin = placement.Origin;
        drawParams.flip = placement.Flip;

        return renderer.DrawTexture(spriteSheet.getSpriteTexture(), drawParams);
    };

    SOP_ASSERT(DrawPlayer(m_Player, m_Player.PlaceholderRect), "Failed to draw player sprite");
}

void Game::RenderEffects(Renderer&) {
    // TODO:
    // attack trails
    // hit sparks
    // dust
    // particles
}
}  // namespace sop
