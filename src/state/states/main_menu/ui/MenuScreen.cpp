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
                ctx.EventDispatcher.Enqueue(
                    NavigationEvent{.Action = NavigationAction::ShowCharacterSelect});
            }),
        builder.Button("Options").Align(Alignment::TopCenter).OnClick([](AppCtx&, ButtonData&) {
            spdlog::info("Options clicked");
        }),
        builder.Button("Scoreboard").Align(Alignment::TopCenter).OnClick([](AppCtx&, ButtonData&) {
            spdlog::info("Scoreboard clicked");
        }),
        builder.Button("Exit").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            ctx.EventDispatcher.Enqueue(ApplicationQuitEvent{});
        }));

    auto root = builder.Align(Alignment::Center, std::move(menu));
    builder.SetRoot(root);
}
}  // namespace sop
