#include "smashorpass/ui/PauseScreen.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

PauseScreen::PauseScreen(EventDispatcher& dispatcher)
    : UIScreen(ApplicationState::Paused, dispatcher) {}

void PauseScreen::Build(UIBuilder& builder) {
    auto actions =
        builder.Column()
            .Spacing(14.0f)
            .Align(Alignment::TopCenter)
            .Add(
                builder.Button("Resume")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher, ButtonData&) {
                        spdlog::info("Resume clicked");
                        dispatcher.Enqueue(ApplicationStateChangeEvent{ApplicationState::Playing});
                    }),

                builder.Button("Options")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher, ButtonData&) {
                    spdlog::info("Options clicked from pause menu");

                    // TODO: Later:
                    // dispatcher.Enqueue(ApplicationStateChangeEvent{
                    //     ApplicationState::Options
                    // });
                }),

                builder.Button("Main Menu")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher, ButtonData&) {
                        spdlog::info("Main menu clicked from pause menu");
                        dispatcher.Enqueue(ApplicationStateChangeEvent{ApplicationState::MainMenu});
                    }),

                builder.Button("Exit")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher, ButtonData&) {
                    spdlog::info("Exit clicked from pause menu");

                    // TODO: Later:
                    dispatcher.Enqueue(ApplicationQuitEvent{});
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