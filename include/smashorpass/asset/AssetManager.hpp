#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "smashorpass/core/ConcurrentQueue.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

template <typename AssetLoadJob, typename RawAssetData>
concept ConvertsToRawAssetData = requires(AppCtx& ctx, AssetLoadJob assetLoadJob) {
    { assetLoadJob.ToRawAssetData(ctx) } -> std::same_as<RawAssetData>;
};

template <typename RawAssetData, typename AssetData>
concept ConvertsToAssetData = requires(AppCtx& ctx, RawAssetData rawAssetData) {
    { rawAssetData.ToAssetData(ctx) } -> std::same_as<AssetData>;
};

template <typename AssetLoadJob, typename AssetData>
concept HasDefaultAssetData = requires(AppCtx& ctx, const AssetLoadJob& assetLoadJob) {
    { AssetData::Default(ctx, assetLoadJob) } -> std::same_as<AssetData>;
};

template <typename AssetDiscoverer, typename AssetLoadJob>
concept DiscoversAssets = requires(AppCtx& ctx) {
    {
        AssetDiscoverer::ListAvailableAssets(ctx)
    } -> std::same_as<Result<std::vector<AssetLoadJob>>>;
};

template <typename AssetDiscoverer,
          typename AssetLoadJob,
          typename RawAssetData,
          typename AssetData>
concept ValidAssetTypeRegistration =
    ConvertsToRawAssetData<AssetLoadJob, RawAssetData> &&
    ConvertsToAssetData<RawAssetData, AssetData> && HasDefaultAssetData<AssetLoadJob, AssetData> &&
    DiscoversAssets<AssetDiscoverer, AssetLoadJob>;

using AssetId = int;

class AssetManager;

template <typename AssetData>
class Asset {
   public:
    Asset() noexcept = default;

    Asset(const Asset&) noexcept = default;
    Asset& operator=(const Asset&) noexcept = default;

    Asset(Asset&&) noexcept = default;
    Asset& operator=(Asset&&) noexcept = default;

    ~Asset() = default;

    [[nodiscard]] AssetId Id() const noexcept {
        return m_Id;
    }

   private:
    friend class AssetManager;

    Asset(AssetId id, ConcurrentQueue<AssetId>& eraseQueue)
        : m_Id(id), m_DropGuard(new int{0}, [id, &eraseQueue](void* ptr) {
              delete static_cast<int*>(ptr);
              eraseQueue.Send(id);
          }) {}

    AssetId m_Id = 0;
    std::shared_ptr<void> m_DropGuard;
};

class AssetManager {
   public:
    explicit AssetManager(AppCtx& ctx);

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    template <typename AssetDiscoverer,
              typename AssetLoadJob,
              typename RawAssetData,
              typename AssetData>
        requires ValidAssetTypeRegistration<AssetDiscoverer, AssetLoadJob, RawAssetData, AssetData>
    Result<void> RegisterAssetType() {
        const TypeInfo assetLoadJobType{typeid(AssetLoadJob)};
        const TypeInfo rawAssetDataType{typeid(RawAssetData)};
        const TypeInfo assetDataType{typeid(AssetData)};
        const TypeInfo assetDiscovererType{typeid(AssetDiscoverer)};

        if (m_AssetTypesByLoadJobType.contains(assetLoadJobType)) {
            return Err(std::string{"Asset load job type is already registered."});
        }
        if (m_AssetTypesByAssetDataType.contains(assetDataType)) {
            return Err(std::string{"Asset data type is already registered."});
        }
        if (m_AssetTypesByDiscovererType.contains(assetDiscovererType)) {
            return Err(std::string{"Asset discoverer type is already registered."});
        }

        AssetType assetType{
            .assetLoadJobType = assetLoadJobType,
            .rawAssetDataType = rawAssetDataType,
            .assetDataType = assetDataType,
            .assetDiscovererType = assetDiscovererType,

            .defaultAssetData = [](AppCtx& ctx, const void* assetLoadJob) -> ErasedPtr {
                const AssetLoadJob& typedAssetLoadJob =
                    *static_cast<const AssetLoadJob*>(assetLoadJob);
                return MakeErased(AssetData::Default(ctx, typedAssetLoadJob));
            },

            .toRawAssetData = [](AppCtx& ctx, void* assetLoadJob) -> ErasedPtr {
                AssetLoadJob& typedAssetLoadJob = *static_cast<AssetLoadJob*>(assetLoadJob);
                return MakeErased(typedAssetLoadJob.ToRawAssetData(ctx));
            },

            .toAssetData = [](AppCtx& ctx, void* rawAssetData) -> ErasedPtr {
                RawAssetData& typedRawAssetData = *static_cast<RawAssetData*>(rawAssetData);
                return MakeErased(typedRawAssetData.ToAssetData(ctx));
            },
        };

        auto [it, inserted] =
            m_AssetTypesByLoadJobType.emplace(assetLoadJobType, std::move(assetType));
        if (!inserted) {
            return Err(std::string{"Failed to register asset type."});
        }

        m_AssetTypesByAssetDataType.emplace(assetDataType, assetLoadJobType);
        m_AssetTypesByDiscovererType.emplace(assetDiscovererType, assetLoadJobType);

        return Ok();
    }

    template <typename AssetDiscoverer, typename AssetLoadJob>
        requires DiscoversAssets<AssetDiscoverer, AssetLoadJob>
    Result<std::vector<AssetLoadJob>> ListAvailableAssets() const {
        const TypeInfo assetLoadJobType{typeid(AssetLoadJob)};
        const TypeInfo assetDiscovererType{typeid(AssetDiscoverer)};

        auto assetTypeIt = m_AssetTypesByLoadJobType.find(assetLoadJobType);
        if (assetTypeIt == m_AssetTypesByLoadJobType.end()) {
            return Err(std::string{"Asset load job type is not registered."});
        }

        const AssetType& assetType = assetTypeIt->second;
        if (assetType.assetDiscovererType != assetDiscovererType) {
            return Err(std::string{"Asset discoverer type does not match asset load job type."});
        }

        return AssetDiscoverer::ListAvailableAssets(m_Ctx);
    }

    void Update();

    template <typename AssetLoadJob, typename AssetData>
    Result<Asset<AssetData>> LoadAsset(AssetLoadJob loadJob) {
        const TypeInfo assetLoadJobType{typeid(AssetLoadJob)};
        const TypeInfo assetDataType{typeid(AssetData)};

        auto assetTypeIt = m_AssetTypesByLoadJobType.find(assetLoadJobType);
        if (assetTypeIt == m_AssetTypesByLoadJobType.end()) {
            return Err(std::string{"Asset load job type is not registered."});
        }

        AssetType& assetType = assetTypeIt->second;
        if (assetType.assetDataType != assetDataType) {
            return Err(std::string{"Asset data type does not match asset load job type."});
        }

        AssetId id = m_NextAssetId++;

        m_StoredAssets.emplace(id,
                               StoredAsset{
                                   .typeInfo = assetType.assetDataType,
                                   .data = assetType.defaultAssetData(m_Ctx, &loadJob),
                                   .actuallyLoaded = false,
                               });

        m_QueueJobs.Send(Job{
            .id = id,
            .assetType = &assetType,
            .assetLoadJob = MakeErased(std::move(loadJob)),
        });

        return Ok(Asset<AssetData>{id, m_EraseQueue});
    }

    template <typename AssetData>
    Result<bool> IsAssetActuallyLoaded(const Asset<AssetData>&) {
        return Err(std::string{"TODO"});
    }

    template <typename AssetData>
    Result<void> WaitUntilActuallyLoaded(const Asset<AssetData>&) {
        return Err(std::string{"TODO"});
    }

    template <typename AssetData>
    Result<std::reference_wrapper<AssetData>> GetAssetData(const Asset<AssetData>& asset) {
        const TypeInfo assetDataType{typeid(AssetData)};

        auto storedAssetIt = m_StoredAssets.find(asset.Id());
        if (storedAssetIt == m_StoredAssets.end()) {
            return Err(std::string{"Asset does not exist."});
        }

        StoredAsset& storedAsset = storedAssetIt->second;
        if (storedAsset.typeInfo != assetDataType) {
            return Err(std::string{"Asset data type mismatch."});
        }

        return Ok(std::ref(AsErased<AssetData>(storedAsset.data)));
    }

   private:
    using TypeInfo = std::type_index;

    struct ErasedDeleter {
        void (*deleter)(void*) = nullptr;

        void operator()(void* ptr) const;
    };

    using ErasedPtr = std::unique_ptr<void, ErasedDeleter>;

    template <typename T>
    static ErasedPtr MakeErased(T value) {
        return ErasedPtr{
            new T(std::move(value)),
            ErasedDeleter{[](void* ptr) { delete static_cast<T*>(ptr); }},
        };
    }

    template <typename T>
    static T& AsErased(ErasedPtr& ptr) {
        return *static_cast<T*>(ptr.get());
    }

    struct AssetType {
        TypeInfo assetLoadJobType{typeid(void)};
        TypeInfo rawAssetDataType{typeid(void)};
        TypeInfo assetDataType{typeid(void)};
        TypeInfo assetDiscovererType{typeid(void)};

        ErasedPtr (*defaultAssetData)(AppCtx& ctx, const void* assetLoadJob) = nullptr;
        ErasedPtr (*toRawAssetData)(AppCtx& ctx, void* assetLoadJob) = nullptr;
        ErasedPtr (*toAssetData)(AppCtx& ctx, void* rawAssetData) = nullptr;
    };

    struct Job {
        AssetId id = 0;
        const AssetType* assetType = nullptr;
        ErasedPtr assetLoadJob;
    };

    struct RawAsset {
        AssetId id = 0;
        const AssetType* assetType = nullptr;
        ErasedPtr rawAssetData;
    };

    class AssetWorker {
       public:
        AssetWorker(AppCtx& ctx,
                    ConcurrentQueue<Job>& queueJobs,
                    ConcurrentQueue<RawAsset>& queueRawAssets);

        ~AssetWorker();

        AssetWorker(const AssetWorker&) = delete;
        AssetWorker& operator=(const AssetWorker&) = delete;

        AssetWorker(AssetWorker&&) = delete;
        AssetWorker& operator=(AssetWorker&&) = delete;

       private:
        void Run(std::stop_token stopToken);

        AppCtx& m_Ctx;
        ConcurrentQueue<Job>& m_QueueJobs;
        ConcurrentQueue<RawAsset>& m_QueueRawAssets;

        std::jthread m_Thread;
    };

    struct StoredAsset {
        TypeInfo typeInfo{typeid(void)};
        ErasedPtr data;
        bool actuallyLoaded = false;
    };

    AppCtx& m_Ctx;

    AssetId m_NextAssetId = 1;

    std::unordered_map<TypeInfo, AssetType> m_AssetTypesByLoadJobType;
    std::unordered_map<TypeInfo, TypeInfo> m_AssetTypesByAssetDataType;
    std::unordered_map<TypeInfo, TypeInfo> m_AssetTypesByDiscovererType;

    ConcurrentQueue<AssetId> m_EraseQueue;
    std::unordered_map<AssetId, StoredAsset> m_StoredAssets;

    ConcurrentQueue<Job> m_QueueJobs;
    ConcurrentQueue<RawAsset> m_QueueRawAssets;

    AssetWorker m_WorkerThread;
};

}  // namespace sop
