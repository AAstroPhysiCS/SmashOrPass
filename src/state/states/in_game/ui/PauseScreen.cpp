#include "smashorpass/state/states/in_game/ui/PauseScreen.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

PauseScreen::PauseScreen(AppCtx& ctx) : UIScreen(ctx) {}

void PauseScreen::Build(UIBuilder& builder) {
    auto actions = builder.Column()
                       .Spacing(14.0f)
                       .Align(Alignment::TopCenter)
                       .Add(builder.Button("Resume")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Resume clicked");
                                    ctx.eventDispatcher.Enqueue(
                                        NavigationEvent{.Action = NavigationAction::ResumeMatch});
                                }),

                            builder.Button("Options")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx&, ButtonData&) {
                                    spdlog::info("Options clicked from pause menu");
                                }),

                            builder.Button("Main Menu")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Main menu clicked from pause menu");
                                    ctx.eventDispatcher.Enqueue(
                                        NavigationEvent{.Action = NavigationAction::ShowMainMenu});
                                }),

                            builder.Button("Exit")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Exit clicked from pause menu");

                                    // TODO: Later:
                                    ctx.eventDispatcher.Enqueue(ApplicationQuitEvent{});
                                }));

    auto menu =
        builder.Column()
            .Spacing(24.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label("PAUSED").Align(Alignment::TopCenter),
                 builder.Label("Take a breath. The fight is waiting.").Align(Alignment::TopCenter),
                 std::move(actions));

    auto root = builder.Align(Alignment::Center, std::move(menu));
    builder.SetRoot(root);
}

}  // namespace sop
