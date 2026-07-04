#include "smashorpass/state/states/main_menu/ui/KeybindSettingsScreen.hpp"

#include <SDL3/SDL_keycode.h>

#include <array>
#include <cstddef>
#include <string>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/persistence/SettingsStore.hpp"
#include "smashorpass/ui/Theme.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

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
    const auto makeBindingButton = [this, &builder](const std::size_t playerIndex,
                                                    const PlayerKeyBindings& bindings,
                                                    const KeybindAction action) {
        const bool isPending =
            m_PendingBinding.has_value() && m_PendingBinding->PlayerIndex == playerIndex &&
            m_PendingBinding->Action == action;

        return builder.Button(isPending ? "Press key..." : KeyName(GetKeyForAction(bindings, action)))
            .Align(Alignment::TopCenter)
            .OnClick([this, playerIndex, action](AppCtx&, ButtonData&) {
                // waits for next keyinput and assigns it to the action
                StartBinding(playerIndex, action);
            });
    };

    const auto makeBindingRows = [&builder, &makeBindingButton](
                                     const std::size_t playerIndex,
                                     const PlayerKeyBindings& bindings) {
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
                .Add(makeBindingButton(playerIndex, bindings, KeybindAction::MoveLeft),
                     makeBindingButton(playerIndex, bindings, KeybindAction::MoveRight),
                     makeBindingButton(playerIndex, bindings, KeybindAction::Jump),
                     makeBindingButton(playerIndex, bindings, KeybindAction::Dash),
                     makeBindingButton(playerIndex, bindings, KeybindAction::Attack));

        return builder.Row()
            .Spacing(28.0f)
            .Align(Alignment::TopCenter)
            .Add(actionColumn, keyColumn);
    };

    const auto makePlayerColumn = [&builder, &makeBindingRows](const char* title,
                                                               const std::size_t playerIndex,
                                                               const PlayerKeyBindings& bindings) {
        return builder.Column()
            .Spacing(12.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label(title)
                     .Font(FontId::Medium)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopCenter),
                 makeBindingRows(playerIndex, bindings));
    };

    const auto& playerBindings = GetAppCtx().settings.PlayerKeyBindingsByPlayer;

    auto players =
        builder.Row()
            .Spacing(56.0f)
            .Align(Alignment::TopCenter)
            .Add(makePlayerColumn("PLAYER 1", 0, playerBindings[0]),
                 makePlayerColumn("PLAYER 2", 1, playerBindings[1]));

    auto actions =
        builder.Row()
            .Spacing(16.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Button("Reset")
                     .Align(Alignment::TopCenter)
                     .OnClick([this](AppCtx& ctx, ButtonData&) { ResetKeybinds(ctx); }),
                 builder.Button("Back")
                     .Align(Alignment::TopCenter)
                     .OnClick([this](AppCtx& ctx, ButtonData&) {
                         m_PendingBinding.reset();
                         ctx.eventDispatcher.Enqueue(
                             NavigationEvent{.Action = NavigationAction::ShowSettings});
                     }));

    auto content =
        builder.Column()
            .Spacing(18.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label("KEYBINDS").Font(FontId::Title).Align(Alignment::TopCenter),
                 players,
                 actions);

    auto root = builder.Align(Alignment::Center, content);
    builder.SetRoot(root);
}

EventFlow KeybindSettingsScreen::OnEvent(AppCtx& ctx, const Event& event) {
    if (!m_PendingBinding.has_value()) {
        return UIScreen::OnEvent(ctx, event);
    }

    // if binding is pending, check input, check edge cases, if valid then assign
    const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload);
    if (keyEvent == nullptr) {
        return UIScreen::OnEvent(ctx, event);
    }

    if (!keyEvent->Down || keyEvent->Repeat) {
        return EventFlow::Consumed;
    }

    if (keyEvent->Key == SDLK_ESCAPE) {
        m_PendingBinding.reset();
        RebuildUI();
        return EventFlow::Consumed;
    }

    AssignPendingBinding(ctx, keyEvent->Key);
    return EventFlow::Consumed;
}

void KeybindSettingsScreen::StartBinding(const std::size_t playerIndex,
                                         const KeybindAction action) {
    m_PendingBinding = PendingBinding{.PlayerIndex = playerIndex, .Action = action};
    RebuildUI();
}

void KeybindSettingsScreen::AssignPendingBinding(AppCtx& ctx, const SDL_Keycode key) {
    if (!m_PendingBinding.has_value() ||
        m_PendingBinding->PlayerIndex >= ctx.settings.PlayerKeyBindingsByPlayer.size()) {
        return;
    }

    PlayerKeyBindings& bindings =
        ctx.settings.PlayerKeyBindingsByPlayer[m_PendingBinding->PlayerIndex];
    if (!CanAssignKey(ctx, m_PendingBinding->PlayerIndex, m_PendingBinding->Action, key)) {
        spdlog::warn("Rejected keybind '{}'", KeyName(key));
        return;
    }

    SetKeyForAction(bindings, m_PendingBinding->Action, key);

    m_PendingBinding.reset();
    SaveSettings(ctx);
    RebuildUI();
}

void KeybindSettingsScreen::ResetKeybinds(AppCtx& ctx) {
    ctx.settings.PlayerKeyBindingsByPlayer = Settings{}.PlayerKeyBindingsByPlayer;
    m_PendingBinding.reset();

    SaveSettings(ctx);
    RebuildUI();
}

bool KeybindSettingsScreen::CanAssignKey(const AppCtx& ctx,
                                         const std::size_t playerIndex,
                                         const KeybindAction action,
                                         const SDL_Keycode key) const {
    if (key == SDLK_UNKNOWN || key == SDLK_ESCAPE || key == SDLK_F1) {
        return false;
    }

    static constexpr std::array kActions{
        KeybindAction::MoveLeft,
        KeybindAction::MoveRight,
        KeybindAction::Jump,
        KeybindAction::Dash,
        KeybindAction::Attack,
    };
    // forbid duplicate key binds
    const auto& playerBindings = ctx.settings.PlayerKeyBindingsByPlayer;
    for (std::size_t currentPlayerIndex = 0; currentPlayerIndex < playerBindings.size();
         ++currentPlayerIndex) {
        const PlayerKeyBindings& bindings = playerBindings[currentPlayerIndex];

        for (const KeybindAction currentAction : kActions) {
            if (currentPlayerIndex == playerIndex && currentAction == action) {
                continue;
            }

            if (GetKeyForAction(bindings, currentAction) == key) {
                return false;
            }
        }
    }

    return true;
}

void KeybindSettingsScreen::SaveSettings(AppCtx& ctx) const {
    if (ctx.settingsPath.empty()) {
        return;
    }

    auto saveResult = SettingsStore::Save(ctx.settingsPath, ctx.settings);
    if (!saveResult) {
        spdlog::warn("Failed to save settings: {}", saveResult.error());
    }
}

void KeybindSettingsScreen::SetKeyForAction(PlayerKeyBindings& bindings,
                                            const KeybindAction action,
                                            const SDL_Keycode key) {
    switch (action) {
        case KeybindAction::MoveLeft:
            bindings.MoveLeft = key;
            return;
        case KeybindAction::MoveRight:
            bindings.MoveRight = key;
            return;
        case KeybindAction::Jump:
            bindings.Jump = key;
            return;
        case KeybindAction::Dash:
            bindings.Dash = key;
            return;
        case KeybindAction::Attack:
            bindings.Attack = key;
            return;
    }
}

SDL_Keycode KeybindSettingsScreen::GetKeyForAction(const PlayerKeyBindings& bindings,
                                                   const KeybindAction action) {
    switch (action) {
        case KeybindAction::MoveLeft:
            return bindings.MoveLeft;
        case KeybindAction::MoveRight:
            return bindings.MoveRight;
        case KeybindAction::Jump:
            return bindings.Jump;
        case KeybindAction::Dash:
            return bindings.Dash;
        case KeybindAction::Attack:
            return bindings.Attack;
    }

    return bindings.MoveLeft;
}

}  // namespace sop
