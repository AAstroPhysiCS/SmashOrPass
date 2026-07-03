#include "smashorpass/state/states/main_menu/ui/SettingsScreen.hpp"

#include <format>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/persistence/SettingsStore.hpp"
#include "smashorpass/ui/Theme.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

SettingsScreen::SettingsScreen(AppCtx& ctx) : UIScreen(ctx) {}

void SettingsScreen::Build(UIBuilder& builder) {
    m_DeathmatchRoundsLabel = g_InvalidWidgetId;
    m_SmashRoundsLabel = g_InvalidWidgetId;
    m_SmashStocksLabel = g_InvalidWidgetId;

    const auto makeControlRow = [this, &builder](const int value, const SettingValue setting) {
        auto valueLabel = builder.Label(std::format("{}", value))
                              .Font(FontId::Medium)
                              .TextColor(Theme::TEXT_PRIMARY_COLOR)
                              .Align(Alignment::Center);

        switch (setting) {
            case SettingValue::DeathmatchRounds:
                m_DeathmatchRoundsLabel = valueLabel.GetId();
                break;
            case SettingValue::SmashRounds:
                m_SmashRoundsLabel = valueLabel.GetId();
                break;
            case SettingValue::SmashStocks:
                m_SmashStocksLabel = valueLabel.GetId();
                break;
        }

        return builder.Row()
            .Spacing(8.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Button("-")
                     .Align(Alignment::Center)
                     .OnClick([this, setting](AppCtx& ctx, ButtonData&) {
                         AdjustSetting(ctx, setting, -1);
                     }),
                 valueLabel,
                 builder.Button("+")
                     .Align(Alignment::Center)
                     .OnClick([this, setting](AppCtx& ctx, ButtonData&) {
                         AdjustSetting(ctx, setting, 1);
                     }));
    };

    auto namesColumn =
        builder.Column()
            .Spacing(16.0f + Theme::BUTTON_PADDING_Y * 2.0f)
            .Align(Alignment::CenterLeft)
            .Add(builder.Label("Deathmatch Rounds")
                     .Font(FontId::Medium)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopLeft),
                 builder.Label("Smash Rounds")
                     .Font(FontId::Medium)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopLeft),
                 builder.Label("Smash Stocks")
                     .Font(FontId::Medium)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopLeft));

    auto controlsColumn =
        builder.Column()
            .Spacing(16.0f)
            .Align(Alignment::TopCenter)
            .Add(makeControlRow(GetAppCtx().settings.DeathmatchRoundsToWin,
                                SettingValue::DeathmatchRounds),
                 makeControlRow(GetAppCtx().settings.SmashRoundsToWin, SettingValue::SmashRounds),
                 makeControlRow(GetAppCtx().settings.SmashStocksPerRound,
                                SettingValue::SmashStocks));

    auto table = builder.Row()
                     .Spacing(40.0f)
                     .Align(Alignment::TopCenter)
                     .Add(namesColumn, controlsColumn);

    auto content =
        builder.Column()
            .Spacing(18.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label("SETTINGS").Font(FontId::Title).Align(Alignment::TopCenter),
                 table,
                 builder.Button("Back")
                     .Align(Alignment::TopCenter)
                     .OnClick([](AppCtx& ctx, ButtonData&) {
                         ctx.eventDispatcher.Enqueue(
                             NavigationEvent{.Action = NavigationAction::ShowMainMenu});
                     }));

    auto root = builder.Align(Alignment::Center, content);
    builder.SetRoot(root);
}

void SettingsScreen::AdjustSetting(AppCtx& ctx, const SettingValue setting, const int delta) {
    switch (setting) {
        case SettingValue::DeathmatchRounds:
            ctx.settings.DeathmatchRoundsToWin += delta;
            break;
        case SettingValue::SmashRounds:
            ctx.settings.SmashRoundsToWin += delta;
            break;
        case SettingValue::SmashStocks:
            ctx.settings.SmashStocksPerRound += delta;
            break;
    }

    ctx.settings.Clamp();
    UpdateSettingLabels();
    SaveSettings(ctx);
}

void SettingsScreen::UpdateSettingLabels() {
    const Settings& settings = GetAppCtx().settings;
    SetValueLabel(m_DeathmatchRoundsLabel, settings.DeathmatchRoundsToWin);
    SetValueLabel(m_SmashRoundsLabel, settings.SmashRoundsToWin);
    SetValueLabel(m_SmashStocksLabel, settings.SmashStocksPerRound);
}

void SettingsScreen::SaveSettings(AppCtx& ctx) const {
    if (ctx.settingsPath.empty()) {
        return;
    }

    auto saveResult = SettingsStore::Save(ctx.settingsPath, ctx.settings);
    if (!saveResult) {
        spdlog::warn("Failed to save settings: {}", saveResult.error());
    }
}

void SettingsScreen::SetValueLabel(const UIWidgetId labelId, const int value) {
    if (labelId == g_InvalidWidgetId) {
        return;
    }

    auto& data = std::get<LabelData>(GetWidgetById(labelId).Data);
    data.Text = std::format("{}", value);
}

}  // namespace sop
