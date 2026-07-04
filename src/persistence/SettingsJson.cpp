#include "smashorpass/persistence/SettingsJson.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>

namespace sop {

namespace {

[[nodiscard]] int KeycodeToJson(const SDL_Keycode key) {
    return static_cast<int>(key);
}

[[nodiscard]] SDL_Keycode KeycodeFromJson(const nlohmann::json& json,
                                          const char* key,
                                          const SDL_Keycode fallback) {
    return static_cast<SDL_Keycode>(json.value(key, KeycodeToJson(fallback)));
}

}  // namespace

void to_json(nlohmann::json& json, const PlayerKeyBindings& bindings) {
    json = nlohmann::json{
        {"move_left", KeycodeToJson(bindings.MoveLeft)},
        {"move_right", KeycodeToJson(bindings.MoveRight)},
        {"jump", KeycodeToJson(bindings.Jump)},
        {"dash", KeycodeToJson(bindings.Dash)},
        {"attack", KeycodeToJson(bindings.Attack)},
    };
}

void from_json(const nlohmann::json& json, PlayerKeyBindings& bindings) {
    const PlayerKeyBindings defaults = bindings;
    bindings.MoveLeft = KeycodeFromJson(json, "move_left", defaults.MoveLeft);
    bindings.MoveRight = KeycodeFromJson(json, "move_right", defaults.MoveRight);
    bindings.Jump = KeycodeFromJson(json, "jump", defaults.Jump);
    bindings.Dash = KeycodeFromJson(json, "dash", defaults.Dash);
    bindings.Attack = KeycodeFromJson(json, "attack", defaults.Attack);
}

void to_json(nlohmann::json& json, const Settings& settings) {
    json = nlohmann::json{
        {"deathmatch_rounds", settings.DeathmatchRoundsToWin},
        {"smash_rounds", settings.SmashRoundsToWin},
        {"smash_stocks", settings.SmashStocksPerRound},
        {"deathmatch_out_of_bounds_damage", settings.DeathmatchOutOfBoundsDamage},
        {"player_key_bindings", settings.PlayerKeyBindingsByPlayer},
    };
}

void from_json(const nlohmann::json& json, Settings& settings) {
    const Settings defaults{};
    settings.DeathmatchRoundsToWin =
        json.value("deathmatch_rounds", defaults.DeathmatchRoundsToWin);
    settings.SmashRoundsToWin = json.value("smash_rounds", defaults.SmashRoundsToWin);
    settings.SmashStocksPerRound = json.value("smash_stocks", defaults.SmashStocksPerRound);
    settings.DeathmatchOutOfBoundsDamage =
        json.value("deathmatch_out_of_bounds_damage", defaults.DeathmatchOutOfBoundsDamage);

    const auto keyBindingsIt = json.find("player_key_bindings");
    if (keyBindingsIt != json.end() && keyBindingsIt->is_array()) {
        const std::size_t bindingCount =
            std::min(settings.PlayerKeyBindingsByPlayer.size(), keyBindingsIt->size());
        for (std::size_t i = 0; i < bindingCount; ++i) {
            keyBindingsIt->at(i).get_to(settings.PlayerKeyBindingsByPlayer[i]);
        }
    }

    settings.Clamp();
}

}  // namespace sop
