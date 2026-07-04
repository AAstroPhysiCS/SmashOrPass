#include "smashorpass/core/AppCtx.hpp"

#include <spdlog/spdlog.h>

#include "smashorpass/persistence/OverallStatsStore.hpp"
#include "smashorpass/persistence/SettingsStore.hpp"
#include "smashorpass/persistence/UserDataPath.hpp"

namespace sop {

AppCtx::AppCtx() : assets(*this), assetRootDir(SOP_ASSET_ROOT_DIR) {}

Result<void> AppCtx::Initialize() {
    TRY_VOID(window.Initialize(
        WindowCreateInfo{.Width = 1920, .Height = 1080, .Title = "Smash Or Pass - The Game"}));
    TRY_VOID(renderer.Initialize(window));
    TRY_VOID(particleSystem.Initialize(renderer));
    TRY_VOID(audioSystem.Initialize());
    TRY_VOID((assets.RegisterAssetType<ArenaAssetDiscoverer,
                                       ArenaAssetLoadJob,
                                       RawArenaAssetData,
                                       ArenaAssetData>()));
    TRY_VOID((assets.RegisterAssetType<CharacterAssetDiscoverer,
                                       CharacterAssetLoadJob,
                                       RawCharacterAssetData,
                                       CharacterAssetData>()));
    TRY_VOID((assets.RegisterAssetType<AudioAssetDiscoverer,
                                       AudioAssetLoadJob,
                                       AudioRawAssetData,
                                       AudioAssetData>()));
    TRY_VOID(InitializeUserData());
    return Ok();
}

Result<void> AppCtx::InitializeUserData() {
    // loads persisted Overall Stats and Settings (Rounds & Keybinds)
    auto userDataPath = UserDataPath::Get();
    if (!userDataPath) {
        spdlog::warn("Failed to resolve user data path: {}", userDataPath.error());
        return Ok();
    }

    userDataDir = std::move(*userDataPath);
    overallStatsPath = userDataDir / "overall_stats.json";
    settingsPath = userDataDir / "settings.json";

    auto loadedStats = OverallStatsStore::Load(overallStatsPath);
    if (!loadedStats) {
        spdlog::warn("Failed to load overall stats: {}", loadedStats.error());
        overallStats = OverallStatsTracker{};
    } else {
        overallStats = std::move(*loadedStats);
    }

    auto loadedSettings = SettingsStore::Load(settingsPath);
    if (!loadedSettings) {
        spdlog::warn("Failed to load settings: {}", loadedSettings.error());
        settings = Settings{};
    } else {
        settings = *loadedSettings;
    }

    return Ok();
}

}  // namespace sop
