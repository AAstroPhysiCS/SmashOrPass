#pragma once

#include <nlohmann/json_fwd.hpp>

#include "smashorpass/core/Settings.hpp"

namespace sop {

void to_json(nlohmann::json& json, const PlayerKeyBindings& bindings);
void from_json(const nlohmann::json& json, PlayerKeyBindings& bindings);
void to_json(nlohmann::json& json, const Settings& settings);
void from_json(const nlohmann::json& json, Settings& settings);

}  // namespace sop
