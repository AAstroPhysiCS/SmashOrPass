#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/state/states/in_game/GameMode.hpp"
#include "smashorpass/ui/UIScreen.hpp"

namespace sop {

class CharacterSelectScreen : public UIScreen {
   public:
    explicit CharacterSelectScreen(AppCtx& ctx);
    virtual ~CharacterSelectScreen() = default;

    void Build(UIBuilder& builder) override;
    void SetGameMode(GameMode mode);

   private:
    std::vector<CharacterAssetLoadJob> m_Characters;
    std::string m_Player1Character;
    std::string m_Player2Character;
    GameMode m_GameMode = GameMode::Smash;

    std::string CharacterName(std::string_view character) const;
    void SelectPlayer1(std::string character);
    void SelectPlayer2(std::string character);
};
}  // namespace sop
