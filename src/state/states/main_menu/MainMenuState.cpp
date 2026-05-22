#include "smashorpass/state/states/main_menu/MainMenuState.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

MainMenuState::MainMenuState(AppCtx& ctx) : m_MainMenuScreen(ctx), m_CharacterSelectScreen(ctx) {
    UIBuilder mainMenuBuilder(m_MainMenuScreen);
    m_MainMenuScreen.Build(mainMenuBuilder);

    UIBuilder characterSelectBuilder(m_CharacterSelectScreen);
    m_CharacterSelectScreen.Build(characterSelectBuilder);
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
    TRY_VOID(ctx.audioSystem.PlayMusic(ctx, MusicDesc{ .Asset = m_MainMenuMusic, .Gain = 0.65f, .Loops = -1, .FadeOutMs = 500 }));

    return Ok();
}

Result<EventFlow> MainMenuState::OnEvent(AppCtx& ctx, const Event& event) {
    if (const auto* navigation = std::get_if<NavigationEvent>(&event.Payload)) {
        switch (navigation->Action) {
            case NavigationAction::ShowMainMenu:
                m_View = View::MainMenu;
                return Ok(EventFlow::Consumed);
            case NavigationAction::ShowCharacterSelect:
                m_View = View::CharacterSelect;
                return Ok(EventFlow::Consumed);
            case NavigationAction::StartMatch:
            case NavigationAction::ResumeMatch:
                ctx.audioSystem.StopBus(AudioBus::Music, 2000);
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
        case View::CharacterSelect:
            return m_CharacterSelectScreen;
    }

    return m_MainMenuScreen;
}

}  // namespace sop
