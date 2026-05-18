#include "smashorpass/state/states/main_menu/MainMenuState.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

using namespace sop_util;

MainMenuState::MainMenuState(AppCtx& ctx) : m_MainMenuScreen(ctx), m_CharacterSelectScreen(ctx) {
    UIBuilder mainMenuBuilder(m_MainMenuScreen);
    m_MainMenuScreen.Build(mainMenuBuilder);

    UIBuilder characterSelectBuilder(m_CharacterSelectScreen);
    m_CharacterSelectScreen.Build(characterSelectBuilder);
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
