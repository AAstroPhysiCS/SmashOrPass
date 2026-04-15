#include "smashorpass/asset/AssetManager.hpp"

namespace sop {
    AssetManager::AssetManager(std::filesystem::path assetRootDir)
        : m_AssetRootDir(std::move(assetRootDir)) {}
}
