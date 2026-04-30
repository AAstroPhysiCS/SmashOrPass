#include "smashorpass/ui/MenuScreen.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

MainMenuScreen::MainMenuScreen(EventDispatcher& dispatcher) : UIScreen(dispatcher) {}

void MainMenuScreen::Build(UIBuilder& builder) {
    auto menu = builder.Column().Spacing(14.0f).Add(
        builder.Label("SMASH OR PASS").Align(Alignment::TopCenter),
        builder.Button("Start Game")
            .Align(Alignment::TopCenter)
            .OnClick([](EventDispatcher& dispatcher) {
                dispatcher.Enqueue(ApplicationStateChangeEvent{ApplicationState::Playing});
            }),
        builder.Button("Options").Align(Alignment::TopCenter).OnClick([](EventDispatcher&) {
            spdlog::info("Options clicked");
        }),
        builder.Button("Scoreboard").Align(Alignment::TopCenter).OnClick([](EventDispatcher&) {
            spdlog::info("Scoreboard clicked");
        }),
        builder.Button("Exit").Align(Alignment::TopCenter).OnClick([](EventDispatcher&) {
            spdlog::info("Exit clicked");
        }));

    auto root = builder.Align(Alignment::Center, std::move(menu));
    builder.SetRoot(root);
}
}  // namespace sop
