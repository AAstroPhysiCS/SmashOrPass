#pragma once

#include "smashorpass/ui/UIScreen.hpp"
#include "smashorpass/asset/AssetManager.hpp"

namespace sop {

class CharacterSelectScreen : public UIScreen {
   public:
    explicit CharacterSelectScreen(EventDispatcher& dispatcher);
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
