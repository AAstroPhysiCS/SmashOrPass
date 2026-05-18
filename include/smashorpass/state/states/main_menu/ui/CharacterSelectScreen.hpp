#pragma once

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class CharacterSelectScreen : public UIScreen {
   public:
    explicit CharacterSelectScreen(AppCtx& ctx);
    virtual ~CharacterSelectScreen() = default;

    void Build(UIBuilder& builder) override;

   private:
    CharacterId m_Player1Character = CharacterId::Samurai;
    CharacterId m_Player2Character = CharacterId::Samurai;

    ButtonData* m_PlayerP1SelectedButtonData = nullptr;
    ButtonData* m_PlayerP2SelectedButtonData = nullptr;

    const char* CharacterName(CharacterId character) const;
    void SelectPlayer1(CharacterId character);
    void SelectPlayer2(CharacterId character);
};
}  // namespace sop
