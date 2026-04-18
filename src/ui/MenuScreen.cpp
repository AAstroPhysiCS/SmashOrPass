#include "smashorpass/ui/MenuScreen.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

    void MenuScreen::OnEvent(const Event& event) {
        UIScreen::OnEvent(event);
    }

    void MenuScreen::Build(UIBuilder& builder) {
        auto menu = builder.Column()
            .Spacing(14.0f)
            .Add(
                builder.Label("SMASH OR PASS").Align(Alignment::TopCenter),
                builder.Button("Start Game").Align(Alignment::TopCenter).OnClick([] {
                    spdlog::info("Start Game clicked");
                }),
                builder.Button("Options").Align(Alignment::TopCenter).OnClick([] {
                    spdlog::info("Options clicked");
                }),
                builder.Button("Scoreboard").Align(Alignment::TopCenter).OnClick([] {
                    spdlog::info("Scoreboard clicked");
                }),
                builder.Button("Exit").Align(Alignment::TopCenter).OnClick([] {
                    spdlog::info("Exit clicked");
                }
            ));

        auto root = builder.Align(Alignment::Center, std::move(menu));
        builder.SetRoot(root);
    }
}  // namespace sop