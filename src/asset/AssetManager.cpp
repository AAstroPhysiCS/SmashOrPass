#include "smashorpass/asset/AssetManager.hpp"

namespace sop {

AssetManager::AssetManager(AppCtx& ctx)
    : m_Ctx(ctx), m_WorkerThread(m_Ctx, m_QueueJobs, m_QueueRawAssets) {}

void AssetManager::Update() {
    while (auto assetId = m_EraseQueue.TryRecv()) {
        m_StoredAssets.erase(*assetId);
    }

    while (auto rawAsset = m_QueueRawAssets.TryRecv()) {
        if (rawAsset->assetType == nullptr) {
            continue;
        }

        auto storedAssetIt = m_StoredAssets.find(rawAsset->id);
        if (storedAssetIt == m_StoredAssets.end()) {
            continue;
        }

        StoredAsset& storedAsset = storedAssetIt->second;
        if (storedAsset.typeInfo != rawAsset->assetType->assetDataType) {
            continue;
        }

        storedAsset.data = rawAsset->assetType->toAssetData(m_Ctx, rawAsset->rawAssetData.get());
        storedAsset.actuallyLoaded = true;
    }
}

void AssetManager::ErasedDeleter::operator()(void* ptr) const {
    if (ptr != nullptr && deleter != nullptr) {
        deleter(ptr);
    }
}

AssetManager::AssetWorker::AssetWorker(AppCtx& ctx,
                                       ConcurrentQueue<Job>& queueJobs,
                                       ConcurrentQueue<RawAsset>& queueRawAssets)
    : m_Ctx(ctx),
      m_QueueJobs(queueJobs),
      m_QueueRawAssets(queueRawAssets),
      m_Thread([this](std::stop_token stopToken) { Run(stopToken); }) {}

AssetManager::AssetWorker::~AssetWorker() {
    m_Thread.request_stop();
    m_QueueJobs.WakeAll();
}

void AssetManager::AssetWorker::Run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        auto job = m_QueueJobs.Recv();
        if (!job) {
            continue;
        }

        ErasedPtr rawAssetData = job->assetType->toRawAssetData(m_Ctx, job->assetLoadJob.get());

        m_QueueRawAssets.Send(RawAsset{
            .id = job->id,
            .assetType = job->assetType,
            .rawAssetData = std::move(rawAssetData),
        });
    }
}

}  // namespace sop
