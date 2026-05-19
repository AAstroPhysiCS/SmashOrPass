#pragma once

/*
 * AssetManager handles runtime discovery, loading, caching, and lifetime
 * management of assets.
 *
 * Asset loading is asynchronous and non-blocking:
 * 1) The caller requests an asset by id.
 * 2) AssetManager creates or reuses a cached slot for that asset.
 * 3) A safe default asset is returned immediately through an AssetHandle.
 * 4) The real asset is loaded and parsed on a background worker thread.
 * 5) On a later AssetManager::Update call, completed loads are converted into
 *    renderer-owned SDL textures and swapped into the cached asset slot.
 * 6) Future AssetManager::Get calls through the same handle return the updated
 *    asset data.
 *
 * AssetHandle instances are reference-counted. When the last handle for an
 * asset is released, the asset is queued for cleanup and removed from the cache
 * during a later update.
 */

#include <SDL3/SDL_surface.h>

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "smashorpass/asset/assets/ArenaAsset.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

template <typename TAsset>
class AssetHandle {
   public:
    AssetHandle() = default;

    bool IsValid() const noexcept {
        return m_Id != nullptr;
    }

    explicit operator bool() const noexcept {
        return IsValid();
    }

   private:
    explicit AssetHandle(std::shared_ptr<const std::string> id) : m_Id(std::move(id)) {}

    std::shared_ptr<const std::string> m_Id;

    friend class AssetManager;
};

using ArenaAssetHandle = AssetHandle<ArenaAsset>;
using CharacterAssetHandle = AssetHandle<CharacterAsset>;

class AssetManager {
   public:
    explicit AssetManager(std::filesystem::path assetRootDir);
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    // Query available assets so everything is runtime and dynamic.
    sop_util::Result<std::vector<std::string>> AvailableArenaAssets(AppCtx& ctx) const;
    sop_util::Result<std::vector<std::string>> AvailableCharacterAssets(AppCtx& ctx) const;

    // Actually load assets.
    sop_util::Result<ArenaAssetHandle> LoadArenaAsset(AppCtx& ctx, std::string_view id);
    sop_util::Result<CharacterAssetHandle> LoadCharacterAsset(AppCtx& ctx, std::string_view id);

    sop_util::Result<std::reference_wrapper<const ArenaAsset>> Get(
        const ArenaAssetHandle& handle) const;
    sop_util::Result<std::reference_wrapper<const CharacterAsset>> Get(
        const CharacterAssetHandle& handle) const;

    void Update(AppCtx& ctx);

   private:
    enum class AssetLoadKind {
        Arena,
        Character,
    };

    using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

    struct LoadedArenaAsset {
        std::string Id;
        SurfacePtr Background{nullptr, SDL_DestroySurface};
        SurfacePtr Foreground{nullptr, SDL_DestroySurface};
        std::vector<SDL_FRect> CollisionBoxes;
    };

    struct LoadedCharacterSpriteSheet {
        CharacterAnimation Animation = CharacterAnimation::Idle;
        SurfacePtr Surface{nullptr, SDL_DestroySurface};
        std::vector<CharacterSpriteSheetFrame> Frames;
    };

    struct LoadedCharacterAsset {
        std::string Id;
        std::unordered_map<CharacterAnimation, LoadedCharacterSpriteSheet> SpriteSheets;
    };

    using LoadedAssetData = std::variant<LoadedArenaAsset, LoadedCharacterAsset>;

    struct AssetLoadRequest {
        AssetLoadKind Kind{};
        std::string Id;
    };

    struct AssetLoadResult {
        AssetLoadKind Kind{};
        std::string Id;
        sop_util::Result<LoadedAssetData> Loaded;
    };

    template <typename TAsset>
    struct AssetSlot {
        explicit AssetSlot(TAsset asset) : Asset(std::move(asset)) {}

        TAsset Asset;
        std::weak_ptr<const std::string> HandleId;
    };

    struct DestructionQueueState {
        std::mutex mutex_destruction_queues;
        std::vector<std::string> ArenaAssetsToDestroy;
        std::vector<std::string> CharacterAssetsToDestroy;
    };

    static sop_util::Result<LoadedArenaAsset> LoadArenaAssetFromDisk(
        const std::filesystem::path& assetRootDir, std::string_view id);
    static sop_util::Result<LoadedCharacterSpriteSheet> LoadCharacterSpriteSheetFromDisk(
        const std::filesystem::path& assetRootDir,
        std::string_view id,
        CharacterAnimation animation);
    static sop_util::Result<LoadedCharacterAsset> LoadCharacterAssetFromDisk(
        const std::filesystem::path& assetRootDir, std::string_view id);

    void WorkerMain(std::stop_token stopToken);
    void QueueLoadRequest(AssetLoadKind kind, std::string id);
    void ProcessLoadedAssetQueue(AppCtx& ctx);
    void ProcessDestructionQueue();

    std::filesystem::path m_AssetRootDir;

    std::shared_ptr<DestructionQueueState> m_DestructionQueues;
    mutable std::mutex m_AssetLifecycleMutex;

    std::unordered_map<std::string, AssetSlot<ArenaAsset>> m_ArenaAssets;
    std::unordered_map<std::string, AssetSlot<CharacterAsset>> m_CharacterAssets;

    std::mutex m_LoadRequestMutex;
    std::condition_variable m_LoadRequestCv;
    std::deque<AssetLoadRequest> m_LoadRequests;

    std::mutex m_LoadResultMutex;
    std::deque<AssetLoadResult> m_LoadResults;

    std::jthread m_LoadWorker;
};

}  // namespace sop
