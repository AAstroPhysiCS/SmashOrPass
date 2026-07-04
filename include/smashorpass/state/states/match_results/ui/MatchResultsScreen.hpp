#pragma once

#include <string>

#include "smashorpass/state/states/in_game/MatchConfig.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class MatchResultsScreen : public UIScreen {
   public:
    explicit MatchResultsScreen(AppCtx& ctx, MatchConfig matchConfig = {});

    void Build(UIBuilder& builder) override;

    void SetResults(const MatchResults& results);

   private:
    void UpdateText();

    [[nodiscard]] std::string MakeWinnerText() const;
    [[nodiscard]] std::string MakeRoundsText() const;
    void SetValueText(UIWidgetId id, int value);

    MatchConfig m_MatchConfig{};
    MatchResults m_Results{};

    UIWidgetId m_WinnerLabel = g_InvalidWidgetId;
    UIWidgetId m_RoundsLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1DamageDealtLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1HitsLandedLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1HeadLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1TorsoLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1LegsLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1StocksLostLabel = g_InvalidWidgetId;
    UIWidgetId m_Player1FallsLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2DamageDealtLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2HitsLandedLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2HeadLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2TorsoLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2LegsLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2StocksLostLabel = g_InvalidWidgetId;
    UIWidgetId m_Player2FallsLabel = g_InvalidWidgetId;
};

}  // namespace sop
