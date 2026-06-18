#include "smashorpass/state/states/main_menu/ui/GameModeSelectScreen.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

GameModeSelectScreen::GameModeSelectScreen(AppCtx& ctx) : UIScreen(ctx) {}

void GameModeSelectScreen::Build(UIBuilder& builder) {
    auto menu = builder.Column().Spacing(14.0f).Add(
        builder.Label("Select Game Mode").Font(FontId::Medium).Align(Alignment::TopCenter),
        builder.Button("Smash").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            ctx.eventDispatcher.Enqueue(NavigationEvent{
                .Action = NavigationAction::ShowCharacterSelect,
                .Mode = GameMode::Smash,
            });
        }),
        builder.Button("Deathmatch")
            .Align(Alignment::TopCenter)
            .OnClick([](AppCtx& ctx, ButtonData&) {
                ctx.eventDispatcher.Enqueue(NavigationEvent{
                    .Action = NavigationAction::ShowCharacterSelect,
                    .Mode = GameMode::Deathmatch,
                });
            }),
        builder.Button("Back").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            spdlog::info("Back clicked");
            ctx.eventDispatcher.Enqueue(NavigationEvent{.Action = NavigationAction::ShowMainMenu});
        }));

    auto root = builder.Align(Alignment::Center, std::move(menu));
    builder.SetRoot(root);

    auto menuSound =
        (GetAppCtx().assets.LoadAsset<AudioAssetLoadJob, AudioAssetData>(AudioAssetLoadJob{
            .Path = std::filesystem::path(SOP_ASSET_ROOT_DIR) / "audio/main_menu.ogg",
            .Type = AudioAssetType::Music,
            .Predecode = false,
        }));

    if (!menuSound) {
        spdlog::warn("Failed to load menu sound effect: {}", menuSound.error());
        return;
    }
}
}  // namespace sop
