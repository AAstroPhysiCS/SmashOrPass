#pragma once

#include <cstdint>
#include <string>

#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

enum class GameMode : uint8_t { Smash, Deathmatch };

struct PlayerMatchState {
    std::string Name;
    int HP = 100;
    int Stocks = 3;
    int RoundsWon = 0;
    bool FacingRight = true;
};

class GameScreen : public UIScreen {
   public:
    GameScreen(AppCtx& ctx);
    virtual ~GameScreen() = default;

    void Build(UIBuilder& builder) final override;
    EventFlow OnEvent(AppCtx& ctx, const Event& event) final override;
    void OnUpdate(AppCtx& ctx) final override;
    sop_util::Result<void> OnRender(AppCtx& ctx) final override;

   private:
    void UpdateHudText();

    std::string MakePlayerText(const PlayerMatchState& player) const;
    std::string MakeCenterText() const;

   private:
    GameMode m_Mode = GameMode::Smash;

    PlayerMatchState m_Player1{"P1", 100, 3, 0, true};
    PlayerMatchState m_Player2{"P2", 100, 3, 0, false};

    int32_t m_TargetRoundsToWin = 3;
    int32_t m_CurrentRound = 1;

    bool m_Paused = false;
    bool m_DebugDraw = false;

    UIWidgetId m_P1Label = g_InvalidWidgetId;
    UIWidgetId m_CenterLabel = g_InvalidWidgetId;
    UIWidgetId m_P2Label = g_InvalidWidgetId;
    UIWidgetId m_BottomHintLabel = g_InvalidWidgetId;
};
}  // namespace sop
