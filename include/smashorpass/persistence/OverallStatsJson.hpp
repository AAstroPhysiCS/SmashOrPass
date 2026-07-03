#pragma once

#include <nlohmann/json_fwd.hpp>

#include "smashorpass/state/states/in_game/MatchStats.hpp"
#include "smashorpass/state/states/in_game/OverallStats.hpp"

namespace sop {

void to_json(nlohmann::json& json, const PlayerMatchStats& stats);
void from_json(const nlohmann::json& json, PlayerMatchStats& stats);

void to_json(nlohmann::json& json, const OverallMatchupStats& stats);
void from_json(const nlohmann::json& json, OverallMatchupStats& stats);

}  // namespace sop
