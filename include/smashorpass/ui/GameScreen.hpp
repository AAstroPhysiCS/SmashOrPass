#pragma once

#include "smashorpass/core/Game.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class GameScreen : public UIScreen {
   public:
    GameScreen(EventDispatcher& dispatcher);
    virtual ~GameScreen() = default;

    void Build(UIBuilder& builder) final override;
    void OnEvent(const Event& event) final override;
    void OnUpdate() final override;
    void OnRender(Renderer& renderer) final override;

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
