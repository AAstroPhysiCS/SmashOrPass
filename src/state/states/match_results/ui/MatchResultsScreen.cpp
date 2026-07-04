#include "smashorpass/state/states/match_results/ui/MatchResultsScreen.hpp"

#include <cmath>
#include <format>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/Theme.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

MatchResultsScreen::MatchResultsScreen(AppCtx& ctx, MatchConfig matchConfig)
    : UIScreen(ctx), m_MatchConfig(std::move(matchConfig)) {}

void MatchResultsScreen::Build(UIBuilder& builder) {
    auto winnerLabel =
        builder.Label("MATCH COMPLETE").Font(FontId::Title).TextColor(Theme::TEXT_PRIMARY_COLOR);
    m_WinnerLabel = winnerLabel.GetId();

    auto roundsLabel =
        builder.Label("Rounds: 0 - 0").Font(FontId::Medium).TextColor(Theme::TEXT_SECONDARY_COLOR);
    m_RoundsLabel = roundsLabel.GetId();

    auto p1DamageDealtLabel = builder.Label("0")
                                  .Font(FontId::Small)
                                  .TextColor(Theme::PLAYER_1_COLOR)
                                  .Align(Alignment::TopRight);
    m_Player1DamageDealtLabel = p1DamageDealtLabel.GetId();

    auto p1HitsLandedLabel = builder.Label("0")
                                 .Font(FontId::Small)
                                 .TextColor(Theme::PLAYER_1_COLOR)
                                 .Align(Alignment::TopRight);
    m_Player1HitsLandedLabel = p1HitsLandedLabel.GetId();

    auto p1HeadLabel = builder.Label("0")
                           .Font(FontId::Small)
                           .TextColor(Theme::PLAYER_1_COLOR)
                           .Align(Alignment::TopRight);
    m_Player1HeadLabel = p1HeadLabel.GetId();

    auto p1TorsoLabel = builder.Label("0")
                            .Font(FontId::Small)
                            .TextColor(Theme::PLAYER_1_COLOR)
                            .Align(Alignment::TopRight);
    m_Player1TorsoLabel = p1TorsoLabel.GetId();

    auto p1LegsLabel = builder.Label("0")
                           .Font(FontId::Small)
                           .TextColor(Theme::PLAYER_1_COLOR)
                           .Align(Alignment::TopRight);
    m_Player1LegsLabel = p1LegsLabel.GetId();

    auto p1FallsLabel = builder.Label("0")
                            .Font(FontId::Small)
                            .TextColor(Theme::PLAYER_1_COLOR)
                            .Align(Alignment::TopRight);
    m_Player1FallsLabel = p1FallsLabel.GetId();

    auto p2DamageDealtLabel = builder.Label("0")
                                  .Font(FontId::Small)
                                  .TextColor(Theme::PLAYER_2_COLOR)
                                  .Align(Alignment::TopRight);
    m_Player2DamageDealtLabel = p2DamageDealtLabel.GetId();

    auto p2HitsLandedLabel = builder.Label("0")
                                 .Font(FontId::Small)
                                 .TextColor(Theme::PLAYER_2_COLOR)
                                 .Align(Alignment::TopRight);
    m_Player2HitsLandedLabel = p2HitsLandedLabel.GetId();

    auto p2HeadLabel = builder.Label("0")
                           .Font(FontId::Small)
                           .TextColor(Theme::PLAYER_2_COLOR)
                           .Align(Alignment::TopRight);
    m_Player2HeadLabel = p2HeadLabel.GetId();

    auto p2TorsoLabel = builder.Label("0")
                            .Font(FontId::Small)
                            .TextColor(Theme::PLAYER_2_COLOR)
                            .Align(Alignment::TopRight);
    m_Player2TorsoLabel = p2TorsoLabel.GetId();

    auto p2LegsLabel = builder.Label("0")
                           .Font(FontId::Small)
                           .TextColor(Theme::PLAYER_2_COLOR)
                           .Align(Alignment::TopRight);
    m_Player2LegsLabel = p2LegsLabel.GetId();

    auto p2FallsLabel = builder.Label("0")
                            .Font(FontId::Small)
                            .TextColor(Theme::PLAYER_2_COLOR)
                            .Align(Alignment::TopRight);
    m_Player2FallsLabel = p2FallsLabel.GetId();

    auto descriptorColumn = builder.Column().Spacing(8.0f).Add(
        builder.Label("Stat").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
        builder.Label("Damage Dealt").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
        builder.Label("Hits Landed").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
        builder.Label("Head").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
        builder.Label("Torso").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR),
        builder.Label("Legs").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR));

    auto player1Column = builder.Column().Spacing(8.0f).Add(builder.Label("P1")
                                                                .Font(FontId::Small)
                                                                .TextColor(Theme::PLAYER_1_COLOR)
                                                                .Align(Alignment::TopRight),
                                                            p1DamageDealtLabel,
                                                            p1HitsLandedLabel,
                                                            p1HeadLabel,
                                                            p1TorsoLabel,
                                                            p1LegsLabel);

    auto player2Column = builder.Column().Spacing(8.0f).Add(builder.Label("P2")
                                                                .Font(FontId::Small)
                                                                .TextColor(Theme::PLAYER_2_COLOR)
                                                                .Align(Alignment::TopRight),
                                                            p2DamageDealtLabel,
                                                            p2HitsLandedLabel,
                                                            p2HeadLabel,
                                                            p2TorsoLabel,
                                                            p2LegsLabel);

    if (m_MatchConfig.Mode != GameMode::Deathmatch) {
        auto stocksLostLabel =
            builder.Label("Stocks Lost").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR);
        descriptorColumn.Add(stocksLostLabel);

        auto p1StocksLostLabel = builder.Label("0")
                                     .Font(FontId::Small)
                                     .TextColor(Theme::PLAYER_1_COLOR)
                                     .Align(Alignment::TopRight);
        m_Player1StocksLostLabel = p1StocksLostLabel.GetId();
        player1Column.Add(p1StocksLostLabel);

        auto p2StocksLostLabel = builder.Label("0")
                                     .Font(FontId::Small)
                                     .TextColor(Theme::PLAYER_2_COLOR)
                                     .Align(Alignment::TopRight);
        m_Player2StocksLostLabel = p2StocksLostLabel.GetId();
        player2Column.Add(p2StocksLostLabel);
    }

    descriptorColumn.Add(
        builder.Label("Falls").Font(FontId::Small).TextColor(Theme::TEXT_PRIMARY_COLOR));
    player1Column.Add(p1FallsLabel);
    player2Column.Add(p2FallsLabel);

    auto table = builder.Row()
                     .Spacing(40.0f)
                     .Align(Alignment::TopCenter)
                     .Add(descriptorColumn, player1Column, player2Column);

    auto actions = builder.Row()
                       .Spacing(16.0f)
                       .Align(Alignment::TopCenter)
                       .Add(builder.Button("Restart Game")
                                .Align(Alignment::TopCenter)
                                .OnClick([this](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Restart game clicked from match results");
                                    ctx.eventDispatcher.Enqueue(NavigationEvent{
                                        .Action = NavigationAction::StartMatch,
                                        .Match = m_MatchConfig,
                                    });
                                }),
                            builder.Button("Main Menu")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Main menu clicked from match results");
                                    ctx.eventDispatcher.Enqueue(
                                        NavigationEvent{.Action = NavigationAction::ShowMainMenu});
                                }),
                            builder.Button("Exit")
                                .Align(Alignment::TopCenter)
                                .OnClick([](AppCtx& ctx, ButtonData&) {
                                    spdlog::info("Exit clicked from match results");
                                    ctx.eventDispatcher.Enqueue(ApplicationQuitEvent{});
                                }));

    auto content = builder.Column()
                       .Spacing(18.0f)
                       .Align(Alignment::TopCenter)
                       .Add(winnerLabel, roundsLabel, table, actions);

    auto root = builder.Align(Alignment::Center, content);
    builder.SetRoot(root);

    UpdateText();
}

void MatchResultsScreen::SetResults(const MatchResults& results) {
    m_Results = results;
    UpdateText();
}

void MatchResultsScreen::UpdateText() {
    if (m_WinnerLabel != g_InvalidWidgetId) {
        auto& data = std::get<LabelData>(GetWidgetById(m_WinnerLabel).Data);
        data.Text = MakeWinnerText();
    }

    if (m_RoundsLabel != g_InvalidWidgetId) {
        auto& data = std::get<LabelData>(GetWidgetById(m_RoundsLabel).Data);
        data.Text = MakeRoundsText();
    }

    SetValueText(m_Player1DamageDealtLabel,
                 static_cast<int>(std::lround(m_Results.Player1Stats.DamageDealt)));
    SetValueText(m_Player1HitsLandedLabel, m_Results.Player1Stats.HitsLanded);
    SetValueText(m_Player1HeadLabel, m_Results.Player1Stats.HeadHitsLanded);
    SetValueText(m_Player1TorsoLabel, m_Results.Player1Stats.TorsoHitsLanded);
    SetValueText(m_Player1LegsLabel, m_Results.Player1Stats.LegHitsLanded);
    SetValueText(m_Player1StocksLostLabel, m_Results.Player1Stats.StocksLost);
    SetValueText(m_Player1FallsLabel, m_Results.Player1Stats.Falls);

    SetValueText(m_Player2DamageDealtLabel,
                 static_cast<int>(std::lround(m_Results.Player2Stats.DamageDealt)));
    SetValueText(m_Player2HitsLandedLabel, m_Results.Player2Stats.HitsLanded);
    SetValueText(m_Player2HeadLabel, m_Results.Player2Stats.HeadHitsLanded);
    SetValueText(m_Player2TorsoLabel, m_Results.Player2Stats.TorsoHitsLanded);
    SetValueText(m_Player2LegsLabel, m_Results.Player2Stats.LegHitsLanded);
    SetValueText(m_Player2StocksLostLabel, m_Results.Player2Stats.StocksLost);
    SetValueText(m_Player2FallsLabel, m_Results.Player2Stats.Falls);
}

std::string MatchResultsScreen::MakeWinnerText() const {
    return std::format("PLAYER {} WINS", m_Results.WinnerIndex + 1);
}

std::string MatchResultsScreen::MakeRoundsText() const {
    return std::format("Rounds: {} - {}", m_Results.Player1RoundsWon, m_Results.Player2RoundsWon);
}

void MatchResultsScreen::SetValueText(const UIWidgetId id, const int value) {
    if (id == g_InvalidWidgetId) {
        return;
    }

    auto& data = std::get<LabelData>(GetWidgetById(id).Data);
    data.Text = std::format("{}", value);
}

}  // namespace sop
