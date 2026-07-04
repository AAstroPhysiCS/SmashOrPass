#pragma once

#include <filesystem>

#include "smashorpass/core/Settings.hpp"
#include "smashorpass/util.hpp"

namespace sop {

class SettingsStore {
   public:
    [[nodiscard]] static Result<Settings> Load(const std::filesystem::path& path);
    [[nodiscard]] static Result<void> Save(const std::filesystem::path& path,
                                           const Settings& settings);
};

}  // namespace sop
