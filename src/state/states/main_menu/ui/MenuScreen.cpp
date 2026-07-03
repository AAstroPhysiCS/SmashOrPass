#include "smashorpass/state/states/main_menu/ui/MenuScreen.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

MainMenuScreen::MainMenuScreen(AppCtx& ctx) : UIScreen(ctx) {}

void MainMenuScreen::Build(UIBuilder& builder) {
    auto menu = builder.Column().Spacing(14.0f).Add(
        builder.Label("SMASH OR PASS").Font(FontId::Title).Align(Alignment::TopCenter),
        builder.Button("Start Game")
            .Align(Alignment::TopCenter)
            .OnClick([](AppCtx& ctx, ButtonData&) {
                ctx.eventDispatcher.Enqueue(
                    NavigationEvent{.Action = NavigationAction::ShowGameModeSelect});
            }),
        builder.Button("Options")
            .Align(Alignment::TopCenter)
            .OnClick([](AppCtx& ctx, ButtonData&) {
                spdlog::info("Options clicked");
                ctx.eventDispatcher.Enqueue(
                    NavigationEvent{.Action = NavigationAction::ShowSettings});
            }),
        builder.Button("Scoreboard")
            .Align(Alignment::TopCenter)
            .OnClick([](AppCtx& ctx, ButtonData&) {
                spdlog::info("Scoreboard clicked");
                ctx.eventDispatcher.Enqueue(
                    NavigationEvent{.Action = NavigationAction::ShowScoreboard});
            }),
        builder.Button("Exit").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            ctx.eventDispatcher.Enqueue(ApplicationQuitEvent{});
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
