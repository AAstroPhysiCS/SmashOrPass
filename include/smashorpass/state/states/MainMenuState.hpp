#pragma once

#include "smashorpass/state/State.hpp"
#include "smashorpass/ui/CharacterSelectScreen.hpp"
#include "smashorpass/ui/MenuScreen.hpp"

namespace sop {

class MainMenuState final : public State {
   public:
    explicit MainMenuState(AppCtx& ctx);
    ~MainMenuState() override = default;

    [[nodiscard]] std::string_view DebugName() const final {
        return "MainMenu";
    }

    Result<EventFlow> OnEvent(AppCtx& ctx, const Event& event) final;
    Result<void> OnUpdate(AppCtx& ctx) final;
    Result<void> OnRender(AppCtx& ctx) final;

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
