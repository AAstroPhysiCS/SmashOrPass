#include "smashorpass/core/AppCtx.hpp"

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
    return Ok();
}

}  // namespace sop
