#include "smashorpass/persistence/OverallStatsStore.hpp"

#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "smashorpass/persistence/OverallStatsJson.hpp"

namespace sop {

namespace {

constexpr int kOverallStatsVersion = 1;

[[nodiscard]] const char* MatchupKey(const MatchupType matchup) {
    switch (matchup) {
        case MatchupType::Player1VsPlayer2:
            return "p1_vs_p2";
        case MatchupType::Player1VsAi:
            return "p1_vs_ai";
    }

    return "";
}

[[nodiscard]] std::string IoError(const std::filesystem::path& path, const char* operation) {
    return std::string{"Failed to "} + operation + " overall stats '" + path.string() + "'";
}

}  // namespace

Result<OverallStatsTracker> OverallStatsStore::Load(const std::filesystem::path& path) {
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error) {
        return Err(std::string{"Failed to check overall stats '"} + path.string() +
                   "': " + error.message());
    }

    if (!exists) {
        return Ok(OverallStatsTracker{});
    }

    std::ifstream file(path);
    if (!file) {
        return Err(IoError(path, "open"));
    }

    try {
        const nlohmann::json json = nlohmann::json::parse(file);
        OverallStatsTracker stats;

        stats.SetStatsFor(MatchupType::Player1VsPlayer2,
                          json.value(MatchupKey(MatchupType::Player1VsPlayer2),
                                     OverallMatchupStats{}));
        stats.SetStatsFor(MatchupType::Player1VsAi,
                          json.value(MatchupKey(MatchupType::Player1VsAi),
                                     OverallMatchupStats{}));

        return Ok(std::move(stats));
    } catch (const nlohmann::json::exception& e) {
        return Err(std::string{"Failed to parse overall stats '"} + path.string() +
                   "': " + e.what());
    }
}

Result<void> OverallStatsStore::Save(const std::filesystem::path& path,
                                     const OverallStatsTracker& stats) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return Err(std::string{"Failed to create stats directory '"} +
                   path.parent_path().string() + "': " + error.message());
    }

    nlohmann::json json{
        {"version", kOverallStatsVersion},
        {MatchupKey(MatchupType::Player1VsPlayer2),
         stats.StatsFor(MatchupType::Player1VsPlayer2)},
        {MatchupKey(MatchupType::Player1VsAi), stats.StatsFor(MatchupType::Player1VsAi)},
    };

    std::ofstream file(path);
    if (!file) {
        return Err(IoError(path, "write"));
    }

    file << json.dump(4) << '\n';
    if (!file) {
        return Err(IoError(path, "write"));
    }

    return Ok();
}

}  // namespace sop
