#include "smashorpass/state/states/main_menu/ui/KeybindSettingsScreen.hpp"

#include <SDL3/SDL_keycode.h>

#include <string>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/Theme.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

namespace {

[[nodiscard]] std::string KeyName(const SDL_Keycode key) {
    const char* name = SDL_GetKeyName(key);
    if (name == nullptr || name[0] == '\0') {
        return "Unbound";
    }
    return std::string{name};
}

}  // namespace

KeybindSettingsScreen::KeybindSettingsScreen(AppCtx& ctx) : UIScreen(ctx) {}

void KeybindSettingsScreen::Build(UIBuilder& builder) {
    const auto makeBindingRows = [&builder](const PlayerKeyBindings& bindings) {
        auto actionColumn =
            builder.Column()
                .Spacing(10.0f + Theme::BUTTON_PADDING_Y * 2.0f)
                .Align(Alignment::CenterLeft)
                .Add(builder.Label("Move Left")
                         .Font(FontId::Small)
                         .TextColor(Theme::TEXT_PRIMARY_COLOR)
                         .Align(Alignment::TopLeft),
                     builder.Label("Move Right")
                         .Font(FontId::Small)
                         .TextColor(Theme::TEXT_PRIMARY_COLOR)
                         .Align(Alignment::TopLeft),
                     builder.Label("Jump")
                         .Font(FontId::Small)
                         .TextColor(Theme::TEXT_PRIMARY_COLOR)
                         .Align(Alignment::TopLeft),
                     builder.Label("Dash")
                         .Font(FontId::Small)
                         .TextColor(Theme::TEXT_PRIMARY_COLOR)
                         .Align(Alignment::TopLeft),
                     builder.Label("Attack")
                         .Font(FontId::Small)
                         .TextColor(Theme::TEXT_PRIMARY_COLOR)
                         .Align(Alignment::TopLeft));

        auto keyColumn =
            builder.Column()
                .Spacing(10.0f)
                .Align(Alignment::TopCenter)
                .Add(builder.Button(KeyName(bindings.MoveLeft)).Align(Alignment::TopCenter),
                     builder.Button(KeyName(bindings.MoveRight)).Align(Alignment::TopCenter),
                     builder.Button(KeyName(bindings.Jump)).Align(Alignment::TopCenter),
                     builder.Button(KeyName(bindings.Dash)).Align(Alignment::TopCenter),
                     builder.Button(KeyName(bindings.Attack)).Align(Alignment::TopCenter));

        return builder.Row().Spacing(28.0f).Align(Alignment::TopCenter).Add(actionColumn, keyColumn);
    };

    const auto makePlayerColumn = [&builder, &makeBindingRows](const char* title,
                                                               const PlayerKeyBindings& bindings) {
        return builder.Column()
            .Spacing(12.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label(title)
                     .Font(FontId::Medium)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopCenter),
                 makeBindingRows(bindings));
    };

    const auto& playerBindings = GetAppCtx().settings.PlayerKeyBindingsByPlayer;

    auto players =
        builder.Row()
            .Spacing(56.0f)
            .Align(Alignment::TopCenter)
            .Add(makePlayerColumn("PLAYER 1", playerBindings[0]),
                 makePlayerColumn("PLAYER 2", playerBindings[1]));

    auto content =
        builder.Column()
            .Spacing(18.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label("KEYBINDS").Font(FontId::Title).Align(Alignment::TopCenter),
                 players,
                 builder.Button("Back")
                     .Align(Alignment::TopCenter)
                     .OnClick([](AppCtx& ctx, ButtonData&) {
                         ctx.eventDispatcher.Enqueue(
                             NavigationEvent{.Action = NavigationAction::ShowSettings});
                     }));

    auto root = builder.Align(Alignment::Center, content);
    builder.SetRoot(root);
}

}  // namespace sop
