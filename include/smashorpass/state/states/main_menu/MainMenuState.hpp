#pragma once

#include "smashorpass/state/State.hpp"
#include "smashorpass/state/states/main_menu/ui/CharacterSelectScreen.hpp"
#include "smashorpass/state/states/main_menu/ui/MenuScreen.hpp"

namespace sop {

class MainMenuState final : public State {
   public:
    explicit MainMenuState(AppCtx& ctx);
    ~MainMenuState() override = default;

    sop_util::Result<void> Initialize(AppCtx& ctx) final;

    [[nodiscard]] std::string_view DebugName() const final {
        return "MainMenu";
    }

    sop_util::Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    sop_util::Result<void> OnUpdate(AppCtx& ctx) final;
    sop_util::Result<void> OnRender(AppCtx& ctx) final;

   private:
    enum class View {
        MainMenu,
        CharacterSelect,
    };

    [[nodiscard]] UIScreen& ActiveScreen();

    View m_View = View::MainMenu;
    MainMenuScreen m_MainMenuScreen;
    CharacterSelectScreen m_CharacterSelectScreen;
};

}  // namespace sop
