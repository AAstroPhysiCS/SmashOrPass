#include "smashorpass/core/Game.hpp"

#include <SDL3/SDL_keycode.h>

#include "PlayerSpritePlacement.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "spdlog/spdlog.h"

namespace sop {

namespace {
constexpr float kPlaceholderWidth = 116.0f;
constexpr float kPlaceholderHeight = 192.0f;
}  // namespace

void Game::OnEvent(const Event& event) {
    EventDispatcher::Dispatch<WindowResizeEvent>(event, [](const WindowResizeEvent&) {
        // Later: update viewport / UI layout / camera projection.
    });
}

void Game::GameplayTick(ApplicationState state) {
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
            //  TODO: update game world, handle player input, etc.
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

    AdvancePlayerAnimation(m_Player1Visual, assetManager);
    AdvancePlayerAnimation(m_Player2Visual, assetManager);
}

void Game::Render(ApplicationState state, Renderer& renderer, AssetManager& assetManager) {
    if (state == ApplicationState::Playing) {
        RenderWorld(renderer, assetManager);
    }
}

void Game::AdvancePlayerAnimation(CharacterVisualState& player, AssetManager& assetManager) {
    const SpriteSheet& spriteSheet =
        assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
    const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
    SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

    player.Animation.Advance(frames.size());
}

void Game::RenderWorld(Renderer& renderer, AssetManager& assetManager) {
    RenderStage(renderer);
    RenderPlayers(renderer, assetManager);
    RenderEffects(renderer);
}

void Game::RenderStage(Renderer& renderer) {
    const SDL_Point size = renderer.GetCurrentOutputSize();

    // Sky/background
    renderer.FillRect(SDL_FRect{0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y)},
                      Color{32, 36, 52, 255});

    // Main platform
    renderer.FillRect(SDL_FRect{static_cast<float>(size.x) * 0.25f,
                                static_cast<float>(size.y) * 0.72f,
                                static_cast<float>(size.x) * 0.50f,
                                24.0f},
                      Color{110, 110, 120, 255});

    renderer.DrawRect(SDL_FRect{static_cast<float>(size.x) * 0.25f,
                                static_cast<float>(size.y) * 0.72f,
                                static_cast<float>(size.x) * 0.50f,
                                24.0f},
                      Color{180, 180, 190, 255});

    // floating platform
    renderer.FillRect(SDL_FRect{static_cast<float>(size.x) * 0.42f,
                                static_cast<float>(size.y) * 0.52f,
                                static_cast<float>(size.x) * 0.16f,
                                16.0f},
                      Color{90, 90, 100, 255});
}

void Game::RenderPlayers(Renderer& renderer, AssetManager& assetManager) {
    const auto DrawPlayer = [&](const CharacterVisualState& player,
                                const SDL_FRect& placeholderRect) {
        const SpriteSheet& spriteSheet =
            assetManager.getSpriteSheet(player.Character, player.Animation.GetAnimation());
        const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
        SOP_ASSERT(!frames.empty(), "Character sprite sheet has no frames");

        const SpriteSheetFrame& frame = frames[player.Animation.GetFrameIndex() % frames.size()];
        const detail::PlayerSpritePlacement placement = detail::MakePlayerSpritePlacement(
            placeholderRect, frame, player.FacingRight, kPlaceholderHeight);

        TextureDrawParams drawParams{};
        drawParams.src = &placement.SourceRect;
        drawParams.dst = placement.DestinationRect;
        drawParams.origin = placement.Origin;
        drawParams.flip = placement.Flip;

        return renderer.DrawTexture(spriteSheet.getSpriteTexture(), drawParams);
    };

    SOP_ASSERT(DrawPlayer(m_Player1Visual,
                          SDL_FRect{260.0f, 420.0f, kPlaceholderWidth, kPlaceholderHeight}),
               "Failed to draw player 1 sprite");
    SOP_ASSERT(DrawPlayer(m_Player2Visual,
                          SDL_FRect{520.0f, 420.0f, kPlaceholderWidth, kPlaceholderHeight}),
               "Failed to draw player 2 sprite");
}

void Game::RenderEffects(Renderer&) {
    // TODO:
    // attack trails
    // hit sparks
    // dust
    // particles
}
}  // namespace sop
