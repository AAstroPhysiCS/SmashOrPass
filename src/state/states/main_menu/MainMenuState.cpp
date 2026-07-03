#include "smashorpass/state/states/main_menu/MainMenuState.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

MainMenuState::MainMenuState(AppCtx& ctx)
    : m_MainMenuScreen(ctx),
      m_GameModeSelectScreen(ctx),
      m_CharacterSelectScreen(ctx),
      m_SettingsScreen(ctx),
      m_ScoreboardScreen(ctx) {
    UIBuilder mainMenuBuilder(m_MainMenuScreen);
    m_MainMenuScreen.Build(mainMenuBuilder);

    UIBuilder gameModeSelectBuilder(m_GameModeSelectScreen);
    m_GameModeSelectScreen.Build(gameModeSelectBuilder);

    UIBuilder characterSelectBuilder(m_CharacterSelectScreen);
    m_CharacterSelectScreen.Build(characterSelectBuilder);

    UIBuilder settingsBuilder(m_SettingsScreen);
    m_SettingsScreen.Build(settingsBuilder);

    UIBuilder scoreboardBuilder(m_ScoreboardScreen);
    m_ScoreboardScreen.Build(scoreboardBuilder);
}

Result<void> MainMenuState::Initialize(AppCtx& ctx) {
    TRY(mainMenuMusic,
        (ctx.assets.LoadAsset<AudioAssetLoadJob, AudioAssetData>(AudioAssetLoadJob{
            .Path = std::filesystem::path(SOP_ASSET_ROOT_DIR) / "audio/main_menu.ogg",
            .Type = AudioAssetType::Music,
            .Predecode = false,
        })));

    m_MainMenuMusic = std::move(mainMenuMusic);

    TRY_VOID(ctx.assets.WaitUntilActuallyLoaded(m_MainMenuMusic));
    TRY_VOID(ctx.audioSystem.PlayMusic(
        ctx, MusicDesc{.asset = m_MainMenuMusic, .Gain = 0.65f, .Loops = -1, .FadeOutMs = 500}));

    return Ok();
}

Result<EventFlow> MainMenuState::OnEvent(AppCtx& ctx, const Event& event) {
    if (const auto* navigation = std::get_if<NavigationEvent>(&event.Payload)) {
        switch (navigation->Action) {
            case NavigationAction::ShowMainMenu:
                m_View = View::MainMenu;
                return Ok(EventFlow::Consumed);
            case NavigationAction::ShowGameModeSelect:
                m_View = View::GameModeSelect;
                return Ok(EventFlow::Consumed);
            case NavigationAction::ShowCharacterSelect:
                m_CharacterSelectScreen.SetMatchConfig(navigation->Match);
                m_View = View::CharacterSelect;
                return Ok(EventFlow::Consumed);
            case NavigationAction::ShowSettings:
                m_SettingsScreen.RebuildUI();
                m_View = View::Settings;
                return Ok(EventFlow::Consumed);
            case NavigationAction::ShowScoreboard:
                m_ScoreboardScreen.RebuildUI();
                m_View = View::Scoreboard;
                return Ok(EventFlow::Consumed);
            case NavigationAction::StartMatch:
            case NavigationAction::ResumeMatch:
            case NavigationAction::ShowMatchResults:
                TRY_VOID(ctx.audioSystem.StopBus(AudioBus::Music, 2000));
                return Ok(EventFlow::Passed);
        }
    }

    return Ok(ActiveScreen().OnEvent(ctx, event));
}

Result<void> MainMenuState::OnUpdate(AppCtx& ctx) {
    ActiveScreen().OnUpdate(ctx);
    return Ok();
}

Result<void> MainMenuState::OnRender(AppCtx& ctx) {
    return ActiveScreen().OnRender(ctx);
}

UIScreen& MainMenuState::ActiveScreen() {
    switch (m_View) {
        case View::MainMenu:
            return m_MainMenuScreen;
        case View::GameModeSelect:
            return m_GameModeSelectScreen;
        case View::CharacterSelect:
            return m_CharacterSelectScreen;
        case View::Settings:
            return m_SettingsScreen;
        case View::Scoreboard:
            return m_ScoreboardScreen;
    }

    return m_MainMenuScreen;
}

}  // namespace sop
