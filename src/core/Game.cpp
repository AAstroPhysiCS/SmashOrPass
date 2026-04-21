#include "smashorpass/core/Game.hpp"

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Event.hpp"

#include <SDL3/SDL_keycode.h>

#include "smashorpass/rendering/Renderer.hpp"

#include "spdlog/spdlog.h"

namespace sop {

    namespace {
        constexpr float kPlaceholderWidth = 44.0f;
        constexpr float kPlaceholderHeight = 72.0f;

        SDL_FRect MakeSourceRect(const SpriteSheetFrame& frame) {
            return SDL_FRect{
                static_cast<float>(frame.x_left),
                static_cast<float>(frame.y_top),
                static_cast<float>(frame.x_right - frame.x_left),
                static_cast<float>(frame.y_bottom - frame.y_top),
            };
        }

        SDL_FRect MakePlayerDestinationRect(const SDL_FRect& placeholderRect, const SpriteSheetFrame& frame) {
            const float scale = kPlaceholderHeight / static_cast<float>(frame.source_h);
            const float width = static_cast<float>(frame.source_w) * scale;
            const float groundY = placeholderRect.y + placeholderRect.h;
            const float centerX = placeholderRect.x + (placeholderRect.w * 0.5f);

            return SDL_FRect{
                centerX - (width * 0.5f),
                groundY - kPlaceholderHeight,
                width,
                kPlaceholderHeight,
            };
        }
    } // namespace

    void Game::OnEvent(const Event& event) {
        EventDispatcher::Dispatch<WindowResizeEvent>(event, [](const WindowResizeEvent& resizeEvent) {
            // Later: update viewport / UI layout / camera projection.
        });
    }

	void Game::Update(ApplicationState state) {
        switch (state) {
            case ApplicationState::MainMenu:
                //spdlog::info("In main menu");
                // TODO: main menu, handle menu input, etc.
                break;
            case ApplicationState::CharacterSelect:
                //spdlog::info("In character select");
                // TODO: character select, handle character select input, etc.
                break;
            case ApplicationState::Playing:
                //spdlog::info("Playing");
                // TODO: update game world, handle player input, etc.
                break;
            case ApplicationState::Paused:
                //spdlog::info("Paused");
                // TODO: pause menu, handle pause menu input, etc.
                break;
            case ApplicationState::GameOver:
                //spdlog::info("Game over");
                // TODO: game over screen, handle game over screen input, etc.
                break;
        }
    }

    void Game::Render(ApplicationState state, Renderer& renderer, AssetManager& assetManager) {
        switch (state) {
            case ApplicationState::Playing: {
                RenderWorld(renderer, assetManager);
                break;
            }
        }
    }
    
    void Game::RenderWorld(Renderer& renderer, AssetManager& assetManager) {
        RenderStage(renderer);
        RenderPlayers(renderer, assetManager);
        RenderEffects(renderer);
    }

    void Game::RenderStage(Renderer& renderer) {
        const SDL_Point size = renderer.GetCurrentOutputSize();

        // Sky/background
        renderer.FillRect(
            SDL_FRect{0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y)},
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
        const SpriteSheet& spriteSheet = assetManager.getSpriteSheet(CharacterId::Robot, CharacterAnimation::Idle);
        const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();
        SOP_ASSERT(!frames.empty(), "Robot idle sprite sheet has no frames");

        const SpriteSheetFrame& frame0 = frames.front();
        const SDL_FRect sourceRect = MakeSourceRect(frame0);

        TextureDrawParams player1Params{};
        player1Params.src = &sourceRect;
        player1Params.dst = MakePlayerDestinationRect(SDL_FRect{260.0f, 420.0f, kPlaceholderWidth, kPlaceholderHeight},
                                                      frame0);
        SOP_ASSERT(renderer.DrawTexture(spriteSheet.getSpriteTexture(), player1Params), "Failed to draw player 1 sprite");

        TextureDrawParams player2Params{};
        player2Params.src = &sourceRect;
        player2Params.dst = MakePlayerDestinationRect(SDL_FRect{520.0f, 420.0f, kPlaceholderWidth, kPlaceholderHeight},
                                                      frame0);
        player2Params.flip = SDL_FLIP_HORIZONTAL;
        SOP_ASSERT(renderer.DrawTexture(spriteSheet.getSpriteTexture(), player2Params), "Failed to draw player 2 sprite");
    }

    void Game::RenderEffects(Renderer& renderer) {
        // TODO:
        // attack trails
        // hit sparks
        // dust
        // particles
    }
}
