#include "smashorpass/ui/MenuScreen.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

    MenuScreen::MenuScreen(EventDispatcher& dispatcher) 
        : UIScreen(dispatcher)
    {
    }

    void MenuScreen::Build(UIBuilder& builder) {
        auto menu = builder.Column()
            .Spacing(14.0f)
            .Add(
                builder.Label("SMASH OR PASS")
                    .Align(Alignment::TopCenter),
                builder.Button("Start Game")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher) {
                        dispatcher.Enqueue(GameStateChangeEvent{GameState::CharacterSelect});
                    }),
                builder.Button("Options")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher) {
                        spdlog::info("Options clicked");
                    }),
                builder.Button("Scoreboard")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher) {
                        spdlog::info("Scoreboard clicked");
                    }),
                builder.Button("Exit")
                    .Align(Alignment::TopCenter)
                    .OnClick([](EventDispatcher& dispatcher) {
                        spdlog::info("Exit clicked");
                    }
            ));

        auto root = builder.Align(Alignment::Center, std::move(menu));
        builder.SetRoot(root);
    }
}  // namespace sop