#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

#include <memory>
#include <string>
#include <vector>

#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

inline constexpr int ARENA_BASELINE_WIDTH = 1920;
inline constexpr int ARENA_BASELINE_HEIGHT = 1080;

inline constexpr float ARENA_BASELINE_ASPECT_RATIO =
    static_cast<float>(ARENA_BASELINE_WIDTH) / static_cast<float>(ARENA_BASELINE_HEIGHT);

using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

struct ArenaAssetLoadJob;
struct RawArenaAssetData;

struct ArenaAssetData {
    std::string m_Id;
    TexturePtr m_Background{nullptr, SDL_DestroyTexture};
    TexturePtr m_Foreground{nullptr, SDL_DestroyTexture};
    std::vector<SDL_FRect> m_CollisionBoxes;

    static ArenaAssetData Default(AppCtx& ctx, const ArenaAssetLoadJob& loadJob);
};

struct ArenaAssetDiscoverer {
    static Result<std::vector<ArenaAssetLoadJob>> ListAvailableAssets(AppCtx& ctx);
};

struct ArenaAssetLoadJob {
    std::string m_Id;

    RawArenaAssetData ToRawAssetData(AppCtx& ctx);
};

struct RawArenaAssetData {
    std::string m_Id;
    std::string m_Error;
    SurfacePtr m_Background{nullptr, SDL_DestroySurface};
    SurfacePtr m_Foreground{nullptr, SDL_DestroySurface};
    std::vector<SDL_FRect> m_CollisionBoxes;

    ArenaAssetData ToAssetData(AppCtx& ctx);
};

}  // namespace sop
