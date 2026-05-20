#include "smashorpass/state/states/main_menu/ui/CharacterSelectScreen.hpp"

#include <cctype>
#include <utility>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/asset/assets/ArenaAsset.hpp"
#include "smashorpass/asset/assets/CharacterAsset.hpp"
#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

CharacterSelectScreen::CharacterSelectScreen(AppCtx& ctx) : UIScreen(ctx) {
    auto availableCharacters =
        ctx.assets.ListAvailableAssets<CharacterAssetDiscoverer, CharacterAssetLoadJob>();
    if (!availableCharacters.has_value()) {
        spdlog::warn("Failed to list character assets: {}", availableCharacters.error());
        return;
    }

    m_Characters = std::move(*availableCharacters);

    if (!m_Characters.empty()) {
        m_Player1Character = m_Characters.front().m_Id;
        m_Player2Character = m_Characters.front().m_Id;
    }
}

void CharacterSelectScreen::Build(UIBuilder& builder) {
    auto backButton =
        builder.Button("Back").Align(Alignment::TopCenter).OnClick([](AppCtx& ctx, ButtonData&) {
            spdlog::info("Back clicked");
            ctx.eventDispatcher.Enqueue(NavigationEvent{.Action = NavigationAction::ShowMainMenu});
        });

    const auto startMatch = [this](AppCtx& ctx, ButtonData&) {
        spdlog::info("Starting match: P1={}, P2={}",
                     CharacterName(m_Player1Character),
                     CharacterName(m_Player2Character));

        auto arenaJobs = ctx.assets.ListAvailableAssets<ArenaAssetDiscoverer, ArenaAssetLoadJob>();
        if (!arenaJobs) {
            spdlog::warn("Failed to list arena assets: {}", arenaJobs.error());
            return;
        }

        ArenaAssetLoadJob arenaJob =
            arenaJobs->empty() ? ArenaAssetLoadJob{} : std::move(arenaJobs->front());
        auto arenaAsset =
            ctx.assets.LoadAsset<ArenaAssetLoadJob, ArenaAssetData>(std::move(arenaJob));
        if (!arenaAsset) {
            spdlog::warn("Failed to load arena: {}", arenaAsset.error());
            return;
        }

        const auto selectedCharacterJob = [this](std::string_view id) {
            for (const CharacterAssetLoadJob& job : m_Characters) {
                if (job.m_Id == id) {
                    return job;
                }
            }
            return CharacterAssetLoadJob{.m_Id = std::string{id}};
        };

        auto player1Asset = ctx.assets.LoadAsset<CharacterAssetLoadJob, CharacterAssetData>(
            selectedCharacterJob(m_Player1Character));
        if (!player1Asset) {
            spdlog::warn("Failed to load player 1 character '{}': {}",
                         m_Player1Character,
                         player1Asset.error());
            return;
        }

        auto player2Asset = ctx.assets.LoadAsset<CharacterAssetLoadJob, CharacterAssetData>(
            selectedCharacterJob(m_Player2Character));
        if (!player2Asset) {
            spdlog::warn("Failed to load player 2 character '{}': {}",
                         m_Player2Character,
                         player2Asset.error());
            return;
        }

        ctx.eventDispatcher.Enqueue(NavigationEvent{
            .Action = NavigationAction::StartMatch,
            .ArenaAsset = *arenaAsset,
            .CharacterAssets = {*player1Asset, *player2Asset},
        });
    };

    if (m_Characters.empty()) {
        auto actions =
            builder.Row()
                .Spacing(16.0f)
                .Align(Alignment::TopCenter)
                .Add(backButton,
                     builder.Button("Start Match").Align(Alignment::TopCenter).OnClick(startMatch));

        auto menu = builder.Column().Spacing(22.0f).Add(
            builder.Label("SELECT YOUR FIGHTERS").Align(Alignment::TopCenter),
            builder.Label("NO CHARACTERS AVAILABLE").Align(Alignment::TopCenter),
            std::move(actions));

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
    for (const CharacterAssetLoadJob& character : m_Characters) {
        auto characterCard = CreateCharacterCard(character.m_Id);
        characterGrid.Add(characterCard);
    }

    auto actions =
        builder.Row()
            .Spacing(16.0f)
            .Align(Alignment::TopCenter)
            .Add(backButton,
                 builder.Button("Start Match").Align(Alignment::TopCenter).OnClick(startMatch));

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
