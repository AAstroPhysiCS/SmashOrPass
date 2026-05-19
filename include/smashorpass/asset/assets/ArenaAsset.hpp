#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <memory>
#include <string>
#include <vector>

namespace sop {

inline constexpr int ARENA_BASELINE_WIDTH = 1920;
inline constexpr int ARENA_BASELINE_HEIGHT = 1080;

inline constexpr float ARENA_BASELINE_ASPECT_RATIO =
    static_cast<float>(ARENA_BASELINE_WIDTH) / static_cast<float>(ARENA_BASELINE_HEIGHT);

using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

struct ArenaAsset {
    std::string m_Id;
    TexturePtr m_Background{nullptr, SDL_DestroyTexture};
    TexturePtr m_Foreground{nullptr, SDL_DestroyTexture};
    // Coordinates are relative to the arena baseline.
    std::vector<SDL_FRect> m_CollisionBoxes;
};

}  // namespace sop
