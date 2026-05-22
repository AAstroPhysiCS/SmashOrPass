#include "smashorpass/asset/assets/ArenaAsset.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

static Result<void> ConfigureTexture(SDL_Texture* texture, std::string_view name) {
    if (texture == nullptr) {
        return Err(std::format("ConfigureTexture failed: {} texture is null", name));
    }

    TRY_VOID(SdlResult(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND),
                       std::format("SDL_SetTextureBlendMode {}", name)));
    TRY_VOID(SdlResult(SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST),
                       std::format("SDL_SetTextureScaleMode {}", name)));
    return Ok();
}

static TexturePtr CreateTextureFromSurface(AppCtx& ctx,
                                           SDL_Surface* surface,
                                           std::string_view name) {
    if (surface == nullptr) {
        spdlog::warn("Cannot create texture for '{}': surface is null", name);
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.renderer.NativeHandle(), surface),
        SDL_DestroyTexture,
    };
    if (!texture) {
        spdlog::warn("Failed to create texture for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    if (auto result = ConfigureTexture(texture.get(), name); !result) {
        spdlog::warn("Failed to configure texture for '{}': {}", name, result.error());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    return texture;
}

static TexturePtr CreateFilledTexture(AppCtx& ctx,
                                      int width,
                                      int height,
                                      Uint8 red,
                                      Uint8 green,
                                      Uint8 blue,
                                      Uint8 alpha,
                                      std::string_view name) {
    SurfacePtr surface{
        SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface,
    };
    if (!surface) {
        spdlog::warn("Failed to create fallback surface for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    if (!SDL_FillSurfaceRect(
            surface.get(), nullptr, SDL_MapSurfaceRGBA(surface.get(), red, green, blue, alpha))) {
        spdlog::warn("Failed to fill fallback surface for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    return CreateTextureFromSurface(ctx, surface.get(), name);
}

static Result<SurfacePtr> LoadSurface(const std::filesystem::path& path, std::string_view name) {
    const std::string pathString = path.string();
    SurfacePtr surface{IMG_Load(pathString.c_str()), SDL_DestroySurface};
    if (!surface) {
        return Err(std::format("Failed to load {} '{}': {}", name, pathString, SDL_GetError()));
    }

    return Ok(std::move(surface));
}

static Result<std::vector<SDL_FRect>> LoadArenaCollisionBoxes(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Err(std::format("Failed to open arena metadata '{}'", path.string()));
    }

    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };

    try {
        const nlohmann::json json = nlohmann::json::parse(file);
        const auto& collisionsJson = json.at("collisions");
        if (!collisionsJson.is_array()) {
            return Err(std::string("Arena metadata collisions field must be an array"));
        }

        std::vector<SDL_FRect> collisionBoxes;
        collisionBoxes.reserve(collisionsJson.size());

        for (const auto& collisionJson : collisionsJson) {
            SDL_FRect box{
                .x = getFloat(collisionJson, "x"),
                .y = getFloat(collisionJson, "y"),
                .w = getFloat(collisionJson, "width"),
                .h = getFloat(collisionJson, "height"),
            };

            if (box.w < 0.0f || box.h < 0.0f) {
                return Err(std::string("Arena collision box cannot be negative"));
            }

            if (box.x < 0.0f || box.y < 0.0f ||
                box.x + box.w > static_cast<float>(ARENA_BASELINE_WIDTH) ||
                box.y + box.h > static_cast<float>(ARENA_BASELINE_HEIGHT)) {
                return Err(std::string("Arena collision box must be inside the arena baseline"));
            }

            collisionBoxes.push_back(box);
        }

        return Ok(std::move(collisionBoxes));
    } catch (const nlohmann::json::exception& e) {
        return Err(std::format("Failed to parse arena metadata '{}': {}", path.string(), e.what()));
    }
}

static Result<std::vector<std::string>> ListDirectoryIds(const std::filesystem::path& dir) {
    std::error_code ec;
    const bool exists = std::filesystem::exists(dir, ec);
    if (ec) {
        return Err(
            std::format("Failed to inspect asset directory '{}': {}", dir.string(), ec.message()));
    }
    if (!exists) {
        return Ok(std::vector<std::string>{});
    }

    std::vector<std::string> ids;
    for (std::filesystem::directory_iterator it{dir, ec}, end; !ec && it != end; it.increment(ec)) {
        if (!it->is_directory(ec) || ec) {
            continue;
        }

        const std::string assetId = it->path().filename().string();
        if (!assetId.empty() && std::ranges::find(ids, assetId) == ids.end()) {
            ids.push_back(assetId);
        }
    }

    if (ec) {
        return Err(
            std::format("Failed to scan asset directory '{}': {}", dir.string(), ec.message()));
    }

    std::ranges::sort(ids);
    return Ok(std::move(ids));
}

ArenaAssetData ArenaAssetData::Default(AppCtx& ctx, const ArenaAssetLoadJob& loadJob) {
    return ArenaAssetData{
        .m_Id = loadJob.m_Id,
        .m_Background = CreateFilledTexture(ctx,
                                            ARENA_BASELINE_WIDTH,
                                            ARENA_BASELINE_HEIGHT,
                                            24,
                                            24,
                                            32,
                                            255,
                                            "fallback arena background"),
        .m_Foreground = CreateFilledTexture(ctx,
                                            ARENA_BASELINE_WIDTH,
                                            ARENA_BASELINE_HEIGHT,
                                            0,
                                            0,
                                            0,
                                            0,
                                            "fallback arena foreground"),
        .m_CollisionBoxes =
            {
                SDL_FRect{.x = 355.0f, .y = 700.0f, .w = 1210.0f, .h = 40.0f},
            },
    };
}

Result<std::vector<ArenaAssetLoadJob>> ArenaAssetDiscoverer::ListAvailableAssets(AppCtx& ctx) {
    TRY(ids, ListDirectoryIds(ctx.assetRootDir / "sprites" / "arenas"));

    std::vector<ArenaAssetLoadJob> jobs;
    jobs.reserve(ids.size());
    for (std::string& id : ids) {
        jobs.push_back(ArenaAssetLoadJob{.m_Id = std::move(id)});
    }

    return Ok(std::move(jobs));
}

RawArenaAssetData ArenaAssetLoadJob::ToRawAssetData(AppCtx& ctx) {
    RawArenaAssetData raw{};
    raw.m_Id = m_Id;
    if (m_Id.empty()) {
        raw.m_Error = "No arena asset id provided. Using fallback arena asset.";
        return raw;
    }

    const std::filesystem::path arenaDir = ctx.assetRootDir / "sprites" / "arenas" / m_Id;

    auto background = LoadSurface(arenaDir / "background.png", "arena background");
    if (!background) {
        raw.m_Error = background.error();
        return raw;
    }

    if ((*background)->w != ARENA_BASELINE_WIDTH || (*background)->h != ARENA_BASELINE_HEIGHT) {
        raw.m_Error = std::format("Arena background '{}' must be {}x{}, got {}x{}",
                                  (arenaDir / "background.png").string(),
                                  ARENA_BASELINE_WIDTH,
                                  ARENA_BASELINE_HEIGHT,
                                  (*background)->w,
                                  (*background)->h);
        return raw;
    }

    auto foreground = LoadSurface(arenaDir / "foreground.png", "arena foreground");
    if (!foreground) {
        raw.m_Error = foreground.error();
        return raw;
    }

    if ((*foreground)->w != ARENA_BASELINE_WIDTH || (*foreground)->h != ARENA_BASELINE_HEIGHT) {
        raw.m_Error = std::format("Arena foreground '{}' must be {}x{}, got {}x{}",
                                  (arenaDir / "foreground.png").string(),
                                  ARENA_BASELINE_WIDTH,
                                  ARENA_BASELINE_HEIGHT,
                                  (*foreground)->w,
                                  (*foreground)->h);
        return raw;
    }

    auto collisionBoxes = LoadArenaCollisionBoxes(arenaDir / "arena.json");
    if (!collisionBoxes) {
        raw.m_Error = collisionBoxes.error();
        return raw;
    }

    raw.m_Background = std::move(*background);
    raw.m_Foreground = std::move(*foreground);
    raw.m_CollisionBoxes = std::move(*collisionBoxes);
    return raw;
}

ArenaAssetData RawArenaAssetData::ToAssetData(AppCtx& ctx) {
    ArenaAssetData fallback = ArenaAssetData::Default(ctx, ArenaAssetLoadJob{.m_Id = m_Id});
    if (!m_Error.empty()) {
        spdlog::warn("Failed to load arena asset '{}': {}", m_Id, m_Error);
        return fallback;
    }

    TexturePtr background = CreateTextureFromSurface(ctx, m_Background.get(), "arena background");
    TexturePtr foreground = CreateTextureFromSurface(ctx, m_Foreground.get(), "arena foreground");
    if (!background || !foreground) {
        spdlog::warn("Failed to create textures for arena asset '{}'. Using fallback.", m_Id);
        return fallback;
    }

    return ArenaAssetData{
        .m_Id = std::move(m_Id),
        .m_Background = std::move(background),
        .m_Foreground = std::move(foreground),
        .m_CollisionBoxes = std::move(m_CollisionBoxes),
    };
}

}  // namespace sop
