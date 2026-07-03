#include "smashorpass/state/states/main_menu/ui/ScoreboardScreen.hpp"

#include <cmath>
#include <format>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/Theme.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

ScoreboardScreen::ScoreboardScreen(AppCtx& ctx) : UIScreen(ctx) {}

void ScoreboardScreen::Build(UIBuilder& builder) {
    const OverallMatchupStats& p1VsP2 =
        GetAppCtx().overallStats.StatsFor(MatchupType::Player1VsPlayer2);
    const OverallMatchupStats& p1VsAi =
        GetAppCtx().overallStats.StatsFor(MatchupType::Player1VsAi);

    auto descriptorColumn =
        builder.Column()
            .Spacing(8.0f)
            .Add(builder.Label("Stat").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("Matches").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("P1 Wins").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("Opponent Wins")
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("P1 Win Rate")
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("P1 Damage")
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("P1 Hits").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("Opponent Damage")
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR),
                 builder.Label("Opponent Hits")
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR));

    const auto makeMatchupColumn = [&](const char* title, const OverallMatchupStats& stats) {
        return builder.Column()
            .Spacing(8.0f)
            .Add(builder.Label(title)
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(std::format("{}", stats.MatchesPlayed))
                     .Font(FontId::Small)
                     .TextColor(Theme::TEXT_PRIMARY_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(std::format("{}", stats.Player1Wins))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_1_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(std::format("{}", stats.OpponentWins))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_2_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(MakeWinRateText(stats))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_1_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(MakeRoundedText(stats.Player1Totals.DamageDealt))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_1_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(std::format("{}", stats.Player1Totals.HitsLanded))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_1_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(MakeRoundedText(stats.OpponentTotals.DamageDealt))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_2_COLOR)
                     .Align(Alignment::TopRight),
                 builder.Label(std::format("{}", stats.OpponentTotals.HitsLanded))
                     .Font(FontId::Small)
                     .TextColor(Theme::PLAYER_2_COLOR)
                     .Align(Alignment::TopRight));
    };

    auto p1VsP2Column = makeMatchupColumn("P1 vs P2", p1VsP2);
    auto p1VsAiColumn = makeMatchupColumn("P1 vs AI", p1VsAi);

    auto table = builder.Row()
                     .Spacing(40.0f)
                     .Align(Alignment::TopCenter)
                     .Add(descriptorColumn, p1VsP2Column, p1VsAiColumn);

    auto content = builder.Column().Spacing(18.0f).Align(Alignment::TopCenter).Add(
        builder.Label("SCOREBOARD").Font(FontId::Title).Align(Alignment::TopCenter),
        table,
        builder.Button("Back").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            spdlog::info("Back clicked from scoreboard");
            ctx.eventDispatcher.Enqueue(NavigationEvent{.Action = NavigationAction::ShowMainMenu});
        }));

    auto root = builder.Align(Alignment::Center, content);
    builder.SetRoot(root);
}

std::string ScoreboardScreen::MakeWinRateText(const OverallMatchupStats& stats) const {
    return std::format("{}%", static_cast<int>(std::lround(stats.Player1WinRate() * 100.0f)));
}

std::string ScoreboardScreen::MakeRoundedText(const float value) const {
    return std::format("{}", static_cast<int>(std::lround(value)));
}

}  // namespace sop
