#include "smashorpass/persistence/SettingsJson.hpp"

#include <nlohmann/json.hpp>

namespace sop {

void to_json(nlohmann::json& json, const Settings& settings) {
    json = nlohmann::json{
        {"deathmatch_rounds", settings.DeathmatchRoundsToWin},
        {"smash_rounds", settings.SmashRoundsToWin},
        {"smash_stocks", settings.SmashStocksPerRound},
    };
}

void from_json(const nlohmann::json& json, Settings& settings) {
    const Settings defaults{};
    settings.DeathmatchRoundsToWin =
        json.value("deathmatch_rounds", defaults.DeathmatchRoundsToWin);
    settings.SmashRoundsToWin = json.value("smash_rounds", defaults.SmashRoundsToWin);
    settings.SmashStocksPerRound = json.value("smash_stocks", defaults.SmashStocksPerRound);
    settings.Clamp();
}

}  // namespace sop
