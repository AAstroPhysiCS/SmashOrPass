#pragma once

#include "smashorpass/asset/assets/AudioAsset.hpp"
#include "smashorpass/state/State.hpp"
#include "smashorpass/state/states/main_menu/ui/CharacterSelectScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/GameModeSelectScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/KeybindSettingsScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/MenuScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/ScoreboardScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/SettingsScreen.hpp"

namespace sop {

class MainMenuState final : public State {
   public:
    explicit MainMenuState(AppCtx& ctx);
    ~MainMenuState() override = default;

    Result<void> Initialize(AppCtx& ctx) final;

    [[nodiscard]] std::string_view DebugName() const final {
        return "MainMenu";
    }

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

   private:
    enum class View {
        MainMenu,
        GameModeSelect,
        CharacterSelect,
        Settings,
        KeybindSettings,
        Scoreboard,
    };

    [[nodiscard]] UIScreen& ActiveScreen();

    View m_View = View::MainMenu;
    MainMenuScreen m_MainMenuScreen;
    GameModeSelectScreen m_GameModeSelectScreen;
    CharacterSelectScreen m_CharacterSelectScreen;
    SettingsScreen m_SettingsScreen;
    KeybindSettingsScreen m_KeybindSettingsScreen;
    ScoreboardScreen m_ScoreboardScreen;

    Asset<AudioAssetData> m_MainMenuMusic;
};

}  // namespace sop
