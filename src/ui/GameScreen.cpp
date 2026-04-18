#include "smashorpass/ui/GameScreen.hpp"

#include <format>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

    GameScreen::GameScreen(EventDispatcher& dispatcher) 
        : UIScreen(dispatcher)
    {
    }

    void GameScreen::Build(UIBuilder& builder) {
        auto p1Label = builder.Label("P1").Font(FontId::Title);
        m_P1Label = p1Label.GetId();

        auto centerLabel = builder.Label("Match").Font(FontId::Title);
        m_CenterLabel = centerLabel.GetId();

        auto p2Label = builder.Label("P2").Font(FontId::Title);
        m_P2Label = p2Label.GetId();

        auto hintLabel = builder.Label("ESC = Pause   F1 = Debug");
        m_BottomHintLabel = hintLabel.GetId();

        /*auto root = builder.Stack().Add(
            builder.Align(Alignment::TopLeft, builder.Column().Spacing(16.0f).Add(p1Label)),
            builder.Align(Alignment::TopCenter, builder.Column().Spacing(16.0f).Add(centerLabel)),
            builder.Align(Alignment::TopRight, builder.Column().Spacing(32.0f).Add(p2Label)),
            builder.Align(Alignment::BottomCenter, builder.Column().Spacing(48.0f).Add(hintLabel)));*/

        auto root = builder.Stack().Add(builder.Align(Alignment::TopLeft, p1Label),
                                        builder.Align(Alignment::TopCenter, centerLabel),
                                        builder.Align(Alignment::TopRight, p2Label),
                                        builder.Align(Alignment::BottomCenter, hintLabel));

        builder.SetRoot(root);

        UpdateHudText();
    }

    void GameScreen::OnEvent(const Event& event) {
        UIScreen::OnEvent(event);

        EventDispatcher::Dispatch<KeyEvent>(event, [this](const KeyEvent& e) {
            if (!e.Down)
                return;
            if (e.Key == SDLK_ESCAPE) {
                m_Paused = !m_Paused;
            }
        });
    }

    void GameScreen::OnUpdate() {
        if (m_Paused) {
            UpdateHudText();
            return;
        }

        // TODO:
        // update player controllers
        // apply gravity / movement
        // resolve collisions
        // resolve hits / knockback / HP
        // advance particles / effects
        // detect round win / match win

        UpdateHudText();
    }

    void GameScreen::OnRender(Renderer& renderer) {
        UIScreen::OnRender(renderer);
    }

    void GameScreen::UpdateHudText() {
        if (m_P1Label != g_InvalidWidgetId) {
            auto& data = std::get<LabelData>(GetWidgetById(m_P1Label).Data);
            data.Text = MakePlayerText(m_Player1);
        }

        if (m_CenterLabel != g_InvalidWidgetId) {
            auto& data = std::get<LabelData>(GetWidgetById(m_CenterLabel).Data);
            data.Text = MakeCenterText();
        }

        if (m_P2Label != g_InvalidWidgetId) {
            auto& data = std::get<LabelData>(GetWidgetById(m_P2Label).Data);
            data.Text = MakePlayerText(m_Player2);
        }

        if (m_BottomHintLabel != g_InvalidWidgetId) {
            auto& data = std::get<LabelData>(GetWidgetById(m_BottomHintLabel).Data);
            data.Text = m_Paused ? "PAUSED   ESC = Resume   F1 = Debug" : "ESC = Pause   F1 = Debug";
        }
    }

    std::string GameScreen::MakePlayerText(const PlayerMatchState& player) const {
        if (m_Mode == GameMode::Deathmatch) {
            return std::format("{}  HP:{}  Rounds:{}", player.Name, player.HP, player.RoundsWon);
        }

        return std::format("{}  Stocks:{}  Rounds:{}", player.Name, player.Stocks, player.RoundsWon);
    }

    std::string GameScreen::MakeCenterText() const {
        const char* modeText = (m_Mode == GameMode::Deathmatch) ? "DEATHMATCH" : "SMASH / SUMO";

        if (m_Paused) {
            return std::format("{}  |  ROUND {}  |  PAUSED", modeText, m_CurrentRound);
        }

        return std::format(
            "{}  |  ROUND {}  |  FIRST TO {}", modeText, m_CurrentRound, m_TargetRoundsToWin);
    }
}  // namespace sop