#include "smashorpass/ui/CharacterSelectScreen.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "smashorpass/asset/AssetManager.hpp"
#include "spdlog/spdlog.h"

namespace sop {

    CharacterSelectScreen::CharacterSelectScreen(EventDispatcher& dispatcher) : UIScreen(ApplicationState::CharacterSelect, dispatcher) {}

    void CharacterSelectScreen::Build(UIBuilder& builder) {

        const auto CreateCharacterCard = [&](CharacterId character,
                                     const char* title,
                                     const char* description) {
            const bool pickedByP1 = m_Player1Character == character;
            const bool pickedByP2 = m_Player2Character == character;

            std::string p1ButtonText = "P1 SELECTED";
            std::string p2ButtonText = "P2 SELECTED";

            return builder.Column().Spacing(8.0f).Align(Alignment::TopCenter)
                .Add(builder.Label(title).Align(Alignment::TopCenter),
                     builder.Label(description).Align(Alignment::TopCenter),
                     builder.Button(p1ButtonText)
                         .Align(Alignment::TopCenter)
                         .OnClick([this, character](EventDispatcher&, ButtonData& d) {
                             SelectPlayer1(character);
                         })
                         .TextColor(pickedByP1 ? Theme::PLAYER_1_COLOR
                                               : Color{255, 255, 255, 255}),
                     builder.Button(p2ButtonText)
                         .Align(Alignment::TopCenter)
                         .OnClick([this, character](EventDispatcher&, ButtonData& d) { 
                             SelectPlayer2(character);
                         })
                         .TextColor(pickedByP2 ? Theme::PLAYER_2_COLOR
                                               : Color{255, 255, 255, 255})
                );
        };

        auto brawlerCard = CreateCharacterCard(CharacterId::Brawler, "BRAWLER", "Balanced fighter");
        auto ninjaCard = CreateCharacterCard(CharacterId::Samurai, "SAMURAI", "Fast and light");
        auto tankCard = CreateCharacterCard(CharacterId::Tank, "TANK", "Heavy hitter");
        auto mageCard = CreateCharacterCard(CharacterId::Mage, "MAGE", "Range and tricks");

        auto characterGrid = builder.Row()
                                 .Spacing(24.0f)
                                 .Align(Alignment::TopCenter)
                                 .Add(
                                      std::move(brawlerCard),
                                      std::move(ninjaCard),
                                      std::move(tankCard),
                                      std::move(mageCard));

        auto actions =
            builder.Row()
                .Spacing(16.0f)
                .Align(Alignment::TopCenter)
                .Add(
                    builder.Button("Back")
                        .Align(Alignment::TopCenter)
                        .OnClick([](EventDispatcher& dispatcher, ButtonData& d) {
                            spdlog::info("Back clicked");
                            dispatcher.Enqueue(ApplicationStateChangeEvent{ApplicationState::MainMenu});
                        }),

                    builder.Button("Start Match")
                        .Align(Alignment::TopCenter)
                        .OnClick([this](EventDispatcher& dispatcher, ButtonData& d) {
                            spdlog::info("Starting match: P1={}, P2={}",
                                         CharacterName(m_Player1Character),
                                         CharacterName(m_Player2Character));

                            dispatcher.Enqueue(ApplicationStateChangeEvent{ApplicationState::Playing});
                        }));

        auto menu = builder.Column().Spacing(22.0f).Add(
            builder.Label("SELECT YOUR FIGHTERS").Align(Alignment::TopCenter),
            std::move(characterGrid),
            std::move(actions));

        auto root = builder.Align(Alignment::Center, std::move(menu));
        builder.SetRoot(root);
    }

    const char* CharacterSelectScreen::CharacterName(CharacterId character) const {
        switch (character) {
            case CharacterId::Samurai:
                return "Samurai";
            case CharacterId::Mage:
                return "Mage";
            case CharacterId::Tank:
                return "Tank";
            case CharacterId::Brawler:
                return "Brawler";
            //TODO: weitere...
        }

        return "Unknown";
    }

    void CharacterSelectScreen::SelectPlayer1(CharacterId character) {
        m_Player1Character = character;
        spdlog::info("Player 1 selected {}", CharacterName(character));

        RebuildUI();
    }

    void CharacterSelectScreen::SelectPlayer2(CharacterId character) {
        m_Player2Character = character;
        spdlog::info("Player 2 selected {}", CharacterName(character));

        RebuildUI();
    }

}  // namespace sop