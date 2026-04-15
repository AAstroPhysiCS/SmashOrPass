#pragma once

#include <filesystem>

namespace sop {
    class AssetManager {
    public:
        explicit AssetManager(std::filesystem::path assetRootDir);

    private:
        std::filesystem::path m_AssetRootDir;
    };
}
