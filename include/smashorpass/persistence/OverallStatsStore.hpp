#pragma once

#include <filesystem>

#include "smashorpass/state/states/in_game/OverallStats.hpp"
#include "smashorpass/util.hpp"

namespace sop {

class OverallStatsStore {
   public:
    [[nodiscard]] static Result<OverallStatsTracker> Load(const std::filesystem::path& path);
    [[nodiscard]] static Result<void> Save(const std::filesystem::path& path,
                                           const OverallStatsTracker& stats);
};

}  // namespace sop
