#include "smashorpass/state/states/main_menu/ui/CharacterSelectScreen.hpp"

#include <cctype>
#include <utility>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

CharacterSelectScreen::CharacterSelectScreen(AppCtx& ctx) : UIScreen(ctx) {
    auto availableCharacters = ctx.Assets.AvailableCharacterAssets(ctx);
    if (!availableCharacters.has_value()) {
        spdlog::warn("Failed to list character assets: {}", availableCharacters.error());
        return;
    }

    m_Characters = std::move(*availableCharacters);
    if (!m_Characters.empty()) {
        m_Player1Character = m_Characters.front();
        m_Player2Character = m_Characters.front();
    }
}

void CharacterSelectScreen::Build(UIBuilder& builder) {
    auto backButton =
        builder.Button("Back").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            spdlog::info("Back clicked");
            ctx.EventDispatcher.Enqueue(
                NavigationEvent{.Action = NavigationAction::ShowMainMenu});
        });

    if (m_Characters.empty()) {
        auto menu = builder.Column().Spacing(22.0f).Add(
            builder.Label("SELECT YOUR FIGHTERS").Align(Alignment::TopCenter),
            builder.Label("NO CHARACTERS AVAILABLE").Align(Alignment::TopCenter),
            backButton);

        auto root = builder.Align(Alignment::Center, std::move(menu));
        builder.SetRoot(root);
        return;
    }

    const auto CreateCharacterCard = [&](const std::string& character) {
        const bool pickedByP1 = m_Player1Character == character;
        const bool pickedByP2 = m_Player2Character == character;

        std::string p1ButtonText = "P1 SELECTED";
        std::string p2ButtonText = "P2 SELECTED";
        std::string title = CharacterName(character);

        return builder.Column()
            .Spacing(8.0f)
            .Align(Alignment::TopCenter)
            .Add(builder.Label(std::move(title)).Align(Alignment::TopCenter),
                 builder.Button(p1ButtonText)
                     .Align(Alignment::TopCenter)
                     .OnClick([this, character](AppCtx&, ButtonData&) { SelectPlayer1(character); })
                     .TextColor(pickedByP1 ? Theme::PLAYER_1_COLOR : Color{255, 255, 255, 255}),
                 builder.Button(p2ButtonText)
                     .Align(Alignment::TopCenter)
                     .OnClick([this, character](AppCtx&, ButtonData&) { SelectPlayer2(character); })
                     .TextColor(pickedByP2 ? Theme::PLAYER_2_COLOR : Color{255, 255, 255, 255}));
    };

    auto characterGrid = builder.Row().Spacing(24.0f).Align(Alignment::TopCenter);
    for (const std::string& character : m_Characters) {
        auto characterCard = CreateCharacterCard(character);
        characterGrid.Add(characterCard);
    }

    auto actions =
        builder.Row()
            .Spacing(16.0f)
            .Align(Alignment::TopCenter)
            .Add(backButton,
                 builder.Button("Start Match")
                     .Align(Alignment::TopCenter)
                     .OnClick([this](AppCtx& ctx, ButtonData&) {
                         spdlog::info("Starting match: P1={}, P2={}",
                                      CharacterName(m_Player1Character),
                                      CharacterName(m_Player2Character));

                         auto arenaIds = ctx.Assets.AvailableArenaAssets(ctx);
                         if (!arenaIds) {
                             spdlog::warn("Failed to list arena assets: {}", arenaIds.error());
                             return;
                         }

                         const std::string arenaId =
                             arenaIds->empty() ? std::string{} : arenaIds->front();
                         auto arenaAsset = ctx.Assets.LoadArenaAsset(ctx, arenaId);
                         if (!arenaAsset) {
                             spdlog::warn(
                                 "Failed to load arena '{}': {}", arenaId, arenaAsset.error());
                             return;
                         }

                         auto player1Asset =
                             ctx.Assets.LoadCharacterAsset(ctx, m_Player1Character);
                         if (!player1Asset) {
                             spdlog::warn("Failed to load player 1 character '{}': {}",
                                          m_Player1Character,
                                          player1Asset.error());
                             return;
                         }

                         auto player2Asset =
                             ctx.Assets.LoadCharacterAsset(ctx, m_Player2Character);
                         if (!player2Asset) {
                             spdlog::warn("Failed to load player 2 character '{}': {}",
                                          m_Player2Character,
                                          player2Asset.error());
                             return;
                         }

                         ctx.EventDispatcher.Enqueue(NavigationEvent{
                             .Action = NavigationAction::StartMatch,
                             .ArenaAsset = *arenaAsset,
                             .CharacterAssets = {*player1Asset, *player2Asset},
                         });
                     }));

    auto menu = builder.Column().Spacing(22.0f).Add(
        builder.Label("SELECT YOUR FIGHTERS").Align(Alignment::TopCenter),
        std::move(characterGrid),
        std::move(actions));

    auto root = builder.Align(Alignment::Center, std::move(menu));
    builder.SetRoot(root);
}

std::string CharacterSelectScreen::CharacterName(std::string_view character) const {
    std::string name{character};
    if (!name.empty()) {
        name.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(name.front())));
    }
    return name;
}

void CharacterSelectScreen::SelectPlayer1(std::string character) {
    spdlog::info("Player 1 selected {}", CharacterName(character));
    m_Player1Character = std::move(character);

    RebuildUI();
}

void CharacterSelectScreen::SelectPlayer2(std::string character) {
    spdlog::info("Player 2 selected {}", CharacterName(character));
    m_Player2Character = std::move(character);

    RebuildUI();
}

}  // namespace sop
