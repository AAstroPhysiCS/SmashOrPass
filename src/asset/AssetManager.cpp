#include "smashorpass/asset/AssetManager.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

static constexpr std::array kCharacterAnimations{
    CharacterAnimation::Idle,
    CharacterAnimation::Walk,
    CharacterAnimation::Ascending,
    CharacterAnimation::Falling,
    CharacterAnimation::Attacks,
    CharacterAnimation::Dash,
};

static std::string_view CharacterAnimationName(CharacterAnimation animation) {
    switch (animation) {
        case CharacterAnimation::Idle:
            return "Idle";
        case CharacterAnimation::Walk:
            return "Walk";
        case CharacterAnimation::Ascending:
            return "Ascending";
        case CharacterAnimation::Falling:
            return "Falling";
        case CharacterAnimation::Attacks:
            return "Attacks";
        case CharacterAnimation::Dash:
            return "Dash";
    }

    return "Unknown";
}

static Result<void> ConfigureArenaTexture(SDL_Texture* texture, std::string_view name) {
    if (texture == nullptr) {
        return Err(std::format("ConfigureArenaTexture failed: {} texture is null", name));
    }

    TRY_VOID(SdlResult(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND),
                       std::string("SDL_SetTextureBlendMode ") + std::string(name)));
    TRY_VOID(SdlResult(SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST),
                       std::string("SDL_SetTextureScaleMode ") + std::string(name)));
    return Ok();
}

static Result<TexturePtr> CreateDefaultArenaTexture(
    AppCtx& ctx, Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha, std::string_view name) {
    SurfacePtr surface{
        SDL_CreateSurface(ARENA_BASELINE_WIDTH, ARENA_BASELINE_HEIGHT, SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface,
    };
    if (!surface) {
        return Err(SdlError(std::string("SDL_CreateSurface ") + std::string(name)));
    }

    TRY_VOID(SdlResult(
        SDL_FillSurfaceRect(
            surface.get(), nullptr, SDL_MapSurfaceRGBA(surface.get(), red, green, blue, alpha)),
        std::string("SDL_FillSurfaceRect ") + std::string(name)));

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.m_Renderer.NativeHandle(), surface.get()),
        SDL_DestroyTexture,
    };
    if (!texture) {
        return Err(SdlError(std::string("SDL_CreateTextureFromSurface ") + std::string(name)));
    }

    TRY_VOID(ConfigureArenaTexture(texture.get(), name));
    return Ok(std::move(texture));
}

static Result<TexturePtr> CreateArenaTextureFromSurface(AppCtx& ctx,
                                                        SDL_Surface* surface,
                                                        std::string_view name) {
    if (surface == nullptr) {
        return Err(std::format("CreateArenaTextureFromSurface failed: {} surface is null", name));
    }

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.m_Renderer.NativeHandle(), surface),
        SDL_DestroyTexture,
    };
    if (!texture) {
        return Err(SdlError(std::string("SDL_CreateTextureFromSurface ") + std::string(name)));
    }

    TRY_VOID(ConfigureArenaTexture(texture.get(), name));
    return Ok(std::move(texture));
}

static Result<ArenaAsset> CreateDefaultArenaAsset(AppCtx& ctx, std::string id) {
    TRY(background, CreateDefaultArenaTexture(ctx, 24, 24, 32, 255, "background"));
    TRY(foreground, CreateDefaultArenaTexture(ctx, 0, 0, 0, 0, "foreground"));

    return Ok(ArenaAsset{
        .m_Id = std::move(id),
        .m_Background = std::move(background),
        .m_Foreground = std::move(foreground),
        .m_CollisionBoxes =
            {
                SDL_FRect{.x = 355.0f, .y = 700.0f, .w = 1210.0f, .h = 40.0f},
            },
    });
}

static Result<void> ConfigureCharacterTexture(SDL_Texture* texture, std::string_view name) {
    if (texture == nullptr) {
        return Err(std::format("ConfigureCharacterTexture failed: {} texture is null", name));
    }

    TRY_VOID(SdlResult(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND),
                       std::string("SDL_SetTextureBlendMode ") + std::string(name)));
    TRY_VOID(SdlResult(SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST),
                       std::string("SDL_SetTextureScaleMode ") + std::string(name)));
    return Ok();
}

static Result<TexturePtr> CreateDefaultCharacterTexture(AppCtx& ctx, std::string_view name) {
    constexpr int kDefaultCharacterWidth = 60;
    constexpr int kDefaultCharacterHeight = 400;

    SurfacePtr surface{
        SDL_CreateSurface(kDefaultCharacterWidth, kDefaultCharacterHeight, SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface,
    };
    if (!surface) {
        return Err(SdlError(std::string("SDL_CreateSurface ") + std::string(name)));
    }

    TRY_VOID(SdlResult(SDL_FillSurfaceRect(
                           surface.get(), nullptr, SDL_MapSurfaceRGBA(surface.get(), 0, 0, 0, 255)),
                       std::string("SDL_FillSurfaceRect ") + std::string(name)));

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.m_Renderer.NativeHandle(), surface.get()),
        SDL_DestroyTexture,
    };
    if (!texture) {
        return Err(SdlError(std::string("SDL_CreateTextureFromSurface ") + std::string(name)));
    }

    TRY_VOID(ConfigureCharacterTexture(texture.get(), name));
    return Ok(std::move(texture));
}

static Result<TexturePtr> CreateCharacterTextureFromSurface(AppCtx& ctx,
                                                            SDL_Surface* surface,
                                                            std::string_view name) {
    if (surface == nullptr) {
        return Err(
            std::format("CreateCharacterTextureFromSurface failed: {} surface is null", name));
    }

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.m_Renderer.NativeHandle(), surface),
        SDL_DestroyTexture,
    };
    if (!texture) {
        return Err(SdlError(std::string("SDL_CreateTextureFromSurface ") + std::string(name)));
    }

    TRY_VOID(ConfigureCharacterTexture(texture.get(), name));
    return Ok(std::move(texture));
}

static Result<CharacterSpriteSheet> CreateDefaultCharacterSpriteSheet(
    AppCtx& ctx, CharacterAnimation animation) {
    TRY(texture, CreateDefaultCharacterTexture(ctx, CharacterAnimationName(animation)));

    CharacterSpriteSheet sheet{};
    sheet.m_Texture = std::move(texture);
    sheet.m_Frames.push_back(CharacterSpriteSheetFrame{
        .m_Location = SDL_FRect{.x = 0.0f, .y = 0.0f, .w = 60.0f, .h = 400.0f},
        .m_Anchor = SDL_Point{.x = 30, .y = 133},
        .m_CollisionBox = SDL_FRect{.x = 0.0f, .y = 0.0f, .w = 60.0f, .h = 400.0f},
    });

    return Ok(std::move(sheet));
}

static Result<CharacterAsset> CreateDefaultCharacterAsset(AppCtx& ctx, std::string id) {
    CharacterAsset asset{};
    asset.m_Id = std::move(id);

    for (const CharacterAnimation animation : kCharacterAnimations) {
        TRY(sheet, CreateDefaultCharacterSpriteSheet(ctx, animation));
        asset.m_SpriteSheets.try_emplace(animation, std::move(sheet));
    }

    return Ok(std::move(asset));
}

static Result<SurfacePtr> LoadArenaSurface(const std::filesystem::path& path,
                                           std::string_view name) {
    const std::string pathString = path.string();
    SurfacePtr surface{IMG_Load(pathString.c_str()), SDL_DestroySurface};
    if (!surface) {
        return Err(
            std::format("Failed to load arena {} '{}': {}", name, pathString, SDL_GetError()));
    }

    if (surface->w != ARENA_BASELINE_WIDTH || surface->h != ARENA_BASELINE_HEIGHT) {
        return Err(std::format("Arena {} '{}' must be {}x{}, got {}x{}",
                               name,
                               pathString,
                               ARENA_BASELINE_WIDTH,
                               ARENA_BASELINE_HEIGHT,
                               surface->w,
                               surface->h));
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

static Result<SurfacePtr> LoadCharacterSurface(const std::filesystem::path& path,
                                               std::string_view name) {
    const std::string pathString = path.string();
    SurfacePtr surface{IMG_Load(pathString.c_str()), SDL_DestroySurface};
    if (!surface) {
        return Err(
            std::format("Failed to load character {} '{}': {}", name, pathString, SDL_GetError()));
    }

    return Ok(std::move(surface));
}

static Result<std::vector<CharacterSpriteSheetFrame>> LoadCharacterFrames(
    const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Err(std::format("Failed to open character metadata '{}'", path.string()));
    }

    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };
    const auto getInt = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<int>();
    };

    try {
        const nlohmann::json json = nlohmann::json::parse(file);
        const auto& framesJson = json.at("frames");
        if (!framesJson.is_array() || framesJson.empty()) {
            return Err(std::string("Character metadata frames field must be a non-empty array"));
        }

        std::vector<CharacterSpriteSheetFrame> frames;
        frames.reserve(framesJson.size());

        for (const auto& frameJson : framesJson) {
            const float xLeft = getFloat(frameJson, "x_left");
            const float xRight = getFloat(frameJson, "x_right");
            const float yTop = getFloat(frameJson, "y_top");
            const float yBottom = getFloat(frameJson, "y_bottom");

            const auto& collisionBoxJson = frameJson.at("collision_box");
            frames.push_back(CharacterSpriteSheetFrame{
                .m_Location =
                    SDL_FRect{.x = xLeft, .y = yTop, .w = xRight - xLeft, .h = yBottom - yTop},
                .m_Anchor = SDL_Point{.x = getInt(frameJson, "anchor_x"),
                                      .y = getInt(frameJson, "anchor_y")},
                .m_CollisionBox =
                    SDL_FRect{
                        .x = getFloat(collisionBoxJson, "x"),
                        .y = getFloat(collisionBoxJson, "y"),
                        .w = getFloat(collisionBoxJson, "width"),
                        .h = getFloat(collisionBoxJson, "height"),
                    },
            });
        }

        return Ok(std::move(frames));
    } catch (const nlohmann::json::exception& e) {
        return Err(
            std::format("Failed to parse character metadata '{}': {}", path.string(), e.what()));
    }
}

AssetManager::AssetManager(std::filesystem::path assetRootDir)
    : m_AssetRootDir(std::move(assetRootDir)),
      m_DestructionQueues(std::make_shared<DestructionQueueState>()),
      m_LoadWorker([this](std::stop_token stopToken) { WorkerMain(stopToken); }) {}

AssetManager::~AssetManager() {
    if (m_LoadWorker.joinable()) {
        m_LoadWorker.request_stop();
        m_LoadRequestCv.notify_all();
    }
}

Result<AssetManager::LoadedArenaAsset> AssetManager::LoadArenaAssetFromDisk(
    const std::filesystem::path& assetRootDir, std::string_view id) {
    if (id.empty()) {
        return Err(std::string("Arena asset id is empty"));
    }

    const std::filesystem::path arenaDir = assetRootDir / "sprites" / "arenas" / std::string{id};
    TRY(background, LoadArenaSurface(arenaDir / "background.png", "background"));
    TRY(foreground, LoadArenaSurface(arenaDir / "foreground.png", "foreground"));
    TRY(collisionBoxes, LoadArenaCollisionBoxes(arenaDir / "arena.json"));

    return Ok(LoadedArenaAsset{
        .Id = std::string{id},
        .Background = std::move(background),
        .Foreground = std::move(foreground),
        .CollisionBoxes = std::move(collisionBoxes),
    });
}

Result<AssetManager::LoadedCharacterSpriteSheet> AssetManager::LoadCharacterSpriteSheetFromDisk(
    const std::filesystem::path& assetRootDir, std::string_view id, CharacterAnimation animation) {
    const std::string animationName{CharacterAnimationName(animation)};
    const std::filesystem::path characterDir =
        assetRootDir / "sprites" / "characters" / std::string{id};

    TRY(surface, LoadCharacterSurface(characterDir / (animationName + ".png"), animationName));
    TRY(frames, LoadCharacterFrames(characterDir / (animationName + ".json")));

    return Ok(LoadedCharacterSpriteSheet{
        .Animation = animation,
        .Surface = std::move(surface),
        .Frames = std::move(frames),
    });
}

Result<AssetManager::LoadedCharacterAsset> AssetManager::LoadCharacterAssetFromDisk(
    const std::filesystem::path& assetRootDir, std::string_view id) {
    if (id.empty()) {
        return Err(std::string("Character asset id is empty"));
    }

    LoadedCharacterAsset asset{};
    asset.Id = std::string{id};

    for (const CharacterAnimation animation : kCharacterAnimations) {
        auto sheet = LoadCharacterSpriteSheetFromDisk(assetRootDir, id, animation);
        if (!sheet) {
            spdlog::warn("Failed to load character asset '{}', animation '{}': {}",
                         id,
                         CharacterAnimationName(animation),
                         sheet.error());
            continue;
        }

        asset.SpriteSheets.try_emplace(animation, std::move(*sheet));
    }

    return Ok(std::move(asset));
}

void AssetManager::WorkerMain(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        AssetLoadRequest request{};
        {
            std::unique_lock lock(m_LoadRequestMutex);
            m_LoadRequestCv.wait(lock, [this, &stopToken] {
                return stopToken.stop_requested() || !m_LoadRequests.empty();
            });

            if (stopToken.stop_requested()) {
                m_LoadRequests.clear();
                return;
            }

            request = std::move(m_LoadRequests.front());
            m_LoadRequests.pop_front();
        }

        Result<LoadedAssetData> loaded = [this, &request]() -> Result<LoadedAssetData> {
            switch (request.Kind) {
                case AssetLoadKind::Arena: {
                    TRY(asset, LoadArenaAssetFromDisk(m_AssetRootDir, request.Id));
                    return Ok(LoadedAssetData{std::move(asset)});
                }
                case AssetLoadKind::Character: {
                    TRY(asset, LoadCharacterAssetFromDisk(m_AssetRootDir, request.Id));
                    return Ok(LoadedAssetData{std::move(asset)});
                }
            }

            return Err(std::string("Unknown asset load request kind"));
        }();

        if (stopToken.stop_requested()) {
            return;
        }

        {
            std::lock_guard lock(m_LoadResultMutex);
            m_LoadResults.push_back(AssetLoadResult{
                .Kind = request.Kind,
                .Id = std::move(request.Id),
                .Loaded = std::move(loaded),
            });
        }
    }
}

void AssetManager::QueueLoadRequest(AssetLoadKind kind, std::string id) {
    {
        std::lock_guard lock(m_LoadRequestMutex);
        m_LoadRequests.push_back(AssetLoadRequest{.Kind = kind, .Id = std::move(id)});
    }

    m_LoadRequestCv.notify_one();
}

void AssetManager::Update(AppCtx& ctx) {
    ProcessLoadedAssetQueue(ctx);
    ProcessDestructionQueue();
}

Result<std::vector<std::string>> AssetManager::AvailableArenaAssets(AppCtx&) const {
    const std::filesystem::path arenaDir = m_AssetRootDir / "sprites" / "arenas";

    std::error_code ec;
    const bool arenaDirExists = std::filesystem::exists(arenaDir, ec);
    if (ec) {
        return Err(std::format(
            "Failed to inspect arena asset directory '{}': {}", arenaDir.string(), ec.message()));
    }
    if (!arenaDirExists) {
        return Ok(std::vector<std::string>{});
    }

    std::vector<std::string> ids;
    for (std::filesystem::directory_iterator it{arenaDir, ec}, end; !ec && it != end;
         it.increment(ec)) {
        if (!it->is_directory(ec) || ec) {
            continue;
        }

        const std::string assetId = it->path().filename().string();
        if (!assetId.empty() && std::ranges::find(ids, assetId) == ids.end()) {
            ids.push_back(assetId);
        }
    }

    if (ec) {
        return Err(std::format(
            "Failed to scan arena asset directory '{}': {}", arenaDir.string(), ec.message()));
    }

    std::ranges::sort(ids);
    return Ok(std::move(ids));
}

Result<std::vector<std::string>> AssetManager::AvailableCharacterAssets(AppCtx&) const {
    const std::filesystem::path characterDir = m_AssetRootDir / "sprites" / "characters";

    std::error_code ec;
    const bool characterDirExists = std::filesystem::exists(characterDir, ec);
    if (ec) {
        return Err(std::format("Failed to inspect character asset directory '{}': {}",
                               characterDir.string(),
                               ec.message()));
    }
    if (!characterDirExists) {
        return Ok(std::vector<std::string>{});
    }

    std::vector<std::string> ids;
    for (std::filesystem::directory_iterator it{characterDir, ec}, end; !ec && it != end;
         it.increment(ec)) {
        if (!it->is_directory(ec) || ec) {
            continue;
        }

        const std::string assetId = it->path().filename().string();
        if (!assetId.empty() && std::ranges::find(ids, assetId) == ids.end()) {
            ids.push_back(assetId);
        }
    }

    if (ec) {
        return Err(std::format("Failed to scan character asset directory '{}': {}",
                               characterDir.string(),
                               ec.message()));
    }

    std::ranges::sort(ids);
    return Ok(std::move(ids));
}

Result<ArenaAssetHandle> AssetManager::LoadArenaAsset(AppCtx& ctx, std::string_view id) {
    const std::string cacheId{id};
    ProcessDestructionQueue();

    std::lock_guard lock(m_AssetLifecycleMutex);

    auto cached = m_ArenaAssets.find(cacheId);
    if (cached == m_ArenaAssets.end()) {
        TRY(asset, CreateDefaultArenaAsset(ctx, cacheId));
        cached = m_ArenaAssets.try_emplace(cacheId, std::move(asset)).first;
        if (cacheId.empty()) {
            spdlog::warn("No arena asset id provided. Using fallback arena asset.");
        } else {
            QueueLoadRequest(AssetLoadKind::Arena, cacheId);
        }
    }

    AssetSlot<ArenaAsset>& slot = cached->second;
    std::shared_ptr<const std::string> handleId = slot.HandleId.lock();
    if (!handleId) {
        std::weak_ptr<DestructionQueueState> destroyQueues = m_DestructionQueues;
        handleId = std::shared_ptr<const std::string>(
            new std::string(cacheId), [destroyQueues](const std::string* idToDestroy) {
                if (auto queues = destroyQueues.lock()) {
                    std::lock_guard queueLock(queues->mutex_destruction_queues);
                    queues->ArenaAssetsToDestroy.push_back(*idToDestroy);
                }
                delete idToDestroy;
            });
        slot.HandleId = handleId;
    }

    return Ok(ArenaAssetHandle{std::move(handleId)});
}

Result<CharacterAssetHandle> AssetManager::LoadCharacterAsset(AppCtx& ctx, std::string_view id) {
    const std::string cacheId{id};
    ProcessDestructionQueue();

    std::lock_guard lock(m_AssetLifecycleMutex);

    auto cached = m_CharacterAssets.find(cacheId);
    if (cached == m_CharacterAssets.end()) {
        TRY(asset, CreateDefaultCharacterAsset(ctx, cacheId));
        cached = m_CharacterAssets.try_emplace(cacheId, std::move(asset)).first;
        if (cacheId.empty()) {
            spdlog::warn("No character asset id provided. Using fallback character asset.");
        } else {
            QueueLoadRequest(AssetLoadKind::Character, cacheId);
        }
    }

    AssetSlot<CharacterAsset>& slot = cached->second;
    std::shared_ptr<const std::string> handleId = slot.HandleId.lock();
    if (!handleId) {
        std::weak_ptr<DestructionQueueState> destroyQueues = m_DestructionQueues;
        handleId = std::shared_ptr<const std::string>(
            new std::string(cacheId), [destroyQueues](const std::string* idToDestroy) {
                if (auto queues = destroyQueues.lock()) {
                    std::lock_guard queueLock(queues->mutex_destruction_queues);
                    queues->CharacterAssetsToDestroy.push_back(*idToDestroy);
                }
                delete idToDestroy;
            });
        slot.HandleId = handleId;
    }

    return Ok(CharacterAssetHandle{std::move(handleId)});
}

Result<std::reference_wrapper<const ArenaAsset>> AssetManager::Get(
    const ArenaAssetHandle& handle) const {
    if (!handle.m_Id) {
        return Err(std::string("AssetManager::Get failed: arena asset handle is invalid"));
    }

    std::lock_guard lock(m_AssetLifecycleMutex);

    const auto asset = m_ArenaAssets.find(*handle.m_Id);
    if (asset == m_ArenaAssets.end()) {
        return Err(
            std::format("AssetManager::Get failed: arena asset '{}' is not loaded", *handle.m_Id));
    }

    if (asset->second.HandleId.lock() != handle.m_Id) {
        return Err(std::format(
            "AssetManager::Get failed: arena asset handle '{}' does not belong to this manager",
            *handle.m_Id));
    }

    return Ok(std::cref(asset->second.Asset));
}

Result<std::reference_wrapper<const CharacterAsset>> AssetManager::Get(
    const CharacterAssetHandle& handle) const {
    if (!handle.m_Id) {
        return Err(std::string("AssetManager::Get failed: character asset handle is invalid"));
    }

    std::lock_guard lock(m_AssetLifecycleMutex);

    const auto asset = m_CharacterAssets.find(*handle.m_Id);
    if (asset == m_CharacterAssets.end()) {
        return Err(std::format("AssetManager::Get failed: character asset '{}' is not loaded",
                               *handle.m_Id));
    }

    if (asset->second.HandleId.lock() != handle.m_Id) {
        return Err(
            std::format("AssetManager::Get failed: character asset handle '{}' does not "
                        "belong to this manager",
                        *handle.m_Id));
    }

    return Ok(std::cref(asset->second.Asset));
}

void AssetManager::ProcessLoadedAssetQueue(AppCtx& ctx) {
    std::deque<AssetLoadResult> loadResults;
    {
        std::lock_guard lock(m_LoadResultMutex);
        loadResults.swap(m_LoadResults);
    }

    for (AssetLoadResult& result : loadResults) {
        switch (result.Kind) {
            case AssetLoadKind::Arena: {
                {
                    std::lock_guard lock(m_AssetLifecycleMutex);
                    const auto asset = m_ArenaAssets.find(result.Id);
                    if (asset == m_ArenaAssets.end() || asset->second.HandleId.expired()) {
                        continue;
                    }
                }

                if (!result.Loaded) {
                    spdlog::warn(
                        "Failed to load arena asset '{}': {}", result.Id, result.Loaded.error());
                    continue;
                }

                LoadedArenaAsset* loaded = std::get_if<LoadedArenaAsset>(&*result.Loaded);
                if (loaded == nullptr) {
                    spdlog::warn("Failed to load arena asset '{}': worker returned character data",
                                 result.Id);
                    continue;
                }

                auto background =
                    CreateArenaTextureFromSurface(ctx, loaded->Background.get(), "background");
                if (!background) {
                    spdlog::warn(
                        "Failed to load arena asset '{}': {}", result.Id, background.error());
                    continue;
                }

                auto foreground =
                    CreateArenaTextureFromSurface(ctx, loaded->Foreground.get(), "foreground");
                if (!foreground) {
                    spdlog::warn(
                        "Failed to load arena asset '{}': {}", result.Id, foreground.error());
                    continue;
                }

                ArenaAsset arenaAsset{
                    .m_Id = std::move(loaded->Id),
                    .m_Background = std::move(*background),
                    .m_Foreground = std::move(*foreground),
                    .m_CollisionBoxes = std::move(loaded->CollisionBoxes),
                };

                std::lock_guard lock(m_AssetLifecycleMutex);
                const auto asset = m_ArenaAssets.find(result.Id);
                if (asset == m_ArenaAssets.end() || asset->second.HandleId.expired()) {
                    continue;
                }

                asset->second.Asset = std::move(arenaAsset);
                break;
            }
            case AssetLoadKind::Character: {
                {
                    std::lock_guard lock(m_AssetLifecycleMutex);
                    const auto asset = m_CharacterAssets.find(result.Id);
                    if (asset == m_CharacterAssets.end() || asset->second.HandleId.expired()) {
                        continue;
                    }
                }

                if (!result.Loaded) {
                    spdlog::warn("Failed to load character asset '{}': {}",
                                 result.Id,
                                 result.Loaded.error());
                    continue;
                }

                LoadedCharacterAsset* loaded = std::get_if<LoadedCharacterAsset>(&*result.Loaded);
                if (loaded == nullptr) {
                    spdlog::warn("Failed to load character asset '{}': worker returned arena data",
                                 result.Id);
                    continue;
                }

                auto characterAssetResult = CreateDefaultCharacterAsset(ctx, std::move(loaded->Id));
                if (!characterAssetResult) {
                    spdlog::warn("Failed to create fallback sheets for character asset '{}': {}",
                                 result.Id,
                                 characterAssetResult.error());
                    continue;
                }

                CharacterAsset characterAsset = std::move(*characterAssetResult);
                for (auto& [animation, loadedSheet] : loaded->SpriteSheets) {
                    auto texture = CreateCharacterTextureFromSurface(
                        ctx, loadedSheet.Surface.get(), CharacterAnimationName(animation));
                    if (!texture) {
                        spdlog::warn(
                            "Failed to create texture for character asset '{}', "
                            "animation '{}': {}",
                            result.Id,
                            CharacterAnimationName(animation),
                            texture.error());
                        continue;
                    }

                    CharacterSpriteSheet sheet{};
                    sheet.m_Texture = std::move(*texture);
                    sheet.m_Frames = std::move(loadedSheet.Frames);
                    characterAsset.m_SpriteSheets.insert_or_assign(animation, std::move(sheet));
                }

                std::lock_guard lock(m_AssetLifecycleMutex);
                const auto asset = m_CharacterAssets.find(result.Id);
                if (asset == m_CharacterAssets.end() || asset->second.HandleId.expired()) {
                    continue;
                }

                asset->second.Asset = std::move(characterAsset);
                break;
            }
        }
    }
}

void AssetManager::ProcessDestructionQueue() {
    std::vector<std::string> arenaIds;
    std::vector<std::string> characterIds;

    {
        std::lock_guard queueLock(m_DestructionQueues->mutex_destruction_queues);
        arenaIds.swap(m_DestructionQueues->ArenaAssetsToDestroy);
        characterIds.swap(m_DestructionQueues->CharacterAssetsToDestroy);
    }

    std::lock_guard lock(m_AssetLifecycleMutex);

    for (const std::string& id : arenaIds) {
        const auto asset = m_ArenaAssets.find(id);
        if (asset != m_ArenaAssets.end() && asset->second.HandleId.expired()) {
            m_ArenaAssets.erase(asset);
        }
    }

    for (const std::string& id : characterIds) {
        const auto asset = m_CharacterAssets.find(id);
        if (asset != m_CharacterAssets.end() && asset->second.HandleId.expired()) {
            m_CharacterAssets.erase(asset);
        }
    }
}

}  // namespace sop
