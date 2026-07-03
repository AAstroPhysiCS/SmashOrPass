#include <algorithm>
#include <cstddef>
#include <vector>

#include "smashorpass/state/states/in_game/Defaults.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"
#include "smashorpass/util.hpp"

namespace sop {

namespace {

[[nodiscard]] float BottomBlastZone(const Arena& arena) {
    return static_cast<float>(arena.dimensions.y + arena.dimensions.h) + kBottomBlastZonePadding;
}

[[nodiscard]] bool IsBelowBlastZone(const Player& player, const Arena& arena) {
    return player.Position().y > BottomBlastZone(arena);
}

[[nodiscard]] bool IsOutOfHealth(const Player& player) {
    return player.Health() <= 0.0f;
}

[[nodiscard]] bool ContainsPlayerIndex(const std::span<const std::size_t> players,
                                       const std::size_t playerIndex) {
    return std::find(players.begin(), players.end(), playerIndex) != players.end();
}

[[nodiscard]] SDL_FPoint RespawnPosition(const std::size_t playerIndex, const Arena& arena) {
    SDL_FPoint spawnPosition = PlayerStartPosition(playerIndex);
    spawnPosition.y = static_cast<float>(arena.dimensions.y) - kRespawnHeightAboveArena;
    return spawnPosition;
}

[[nodiscard]] bool FacingRightAtSpawn(const SDL_FPoint spawnPosition, const Arena& arena) {
    return spawnPosition.x < static_cast<float>(arena.dimensions.w) * 0.5f;
}

}  // namespace

Result<void> InGameState::ResolveDeathsAndRespawns(AppCtx& ctx) {
    std::vector<std::size_t> blastZonePlayers;
    std::vector<std::size_t> outOfHealthPlayers;
    for (std::size_t playerIndex = 0; playerIndex < m_Players.size(); ++playerIndex) {
        if (IsBelowBlastZone(m_Players[playerIndex], m_Arena)) {
            blastZonePlayers.push_back(playerIndex);
        }

        if (IsOutOfHealth(m_Players[playerIndex])) {
            outOfHealthPlayers.push_back(playerIndex);
        }
    }

    if (blastZonePlayers.empty() && outOfHealthPlayers.empty()) {
        return Ok();
    }

    if (m_MatchConfig.Mode == GameMode::Deathmatch) {
        ResolveDeathmatchDeaths(ctx, blastZonePlayers, outOfHealthPlayers);
        return Ok();
    }

    ResolveSmashDeaths(ctx, blastZonePlayers);
    return Ok();
}

void InGameState::ResolveDeathmatchDeaths(AppCtx& ctx,
                                          const std::span<const std::size_t> blastZonePlayers,
                                          const std::span<const std::size_t> outOfHealthPlayers) {
    std::vector<std::size_t> defeatedPlayers;
    defeatedPlayers.reserve(blastZonePlayers.size() + outOfHealthPlayers.size());
    for (const std::size_t playerIndex : outOfHealthPlayers) {
        defeatedPlayers.push_back(playerIndex);
    }
    for (const std::size_t playerIndex : blastZonePlayers) {
        if (!ContainsPlayerIndex(defeatedPlayers, playerIndex)) {
            defeatedPlayers.push_back(playerIndex);
        }
    }

    for (const std::size_t playerIndex : defeatedPlayers) {
        RecordPlayerDefeat(playerIndex, false);
    }

    if (m_Players.size() == 2 && !outOfHealthPlayers.empty()) {
        const bool player1Out = ContainsPlayerIndex(outOfHealthPlayers, 0);
        const bool player2Out = ContainsPlayerIndex(outOfHealthPlayers, 1);

        if (TryResolveTwoPlayerRoundEnd(ctx, player1Out, player2Out)) {
            return;
        }
    }

    for (const std::size_t playerIndex : outOfHealthPlayers) {
        RespawnPlayerAtArenaSpawn(playerIndex, kDefaultPlayerHealth);
    }

    for (const std::size_t playerIndex : blastZonePlayers) {
        if (!ContainsPlayerIndex(outOfHealthPlayers, playerIndex)) {
            RespawnPlayerAtArenaSpawn(playerIndex, m_Players[playerIndex].Health());
        }
    }
}

void InGameState::ResolveSmashDeaths(AppCtx& ctx,
                                     const std::span<const std::size_t> blastZonePlayers) {
    if (blastZonePlayers.empty()) {
        return;
    }

    for (const std::size_t playerIndex : blastZonePlayers) {
        RecordPlayerDefeat(playerIndex, true);
        m_Players[playerIndex].LoseStock();
    }

    if (m_Players.size() == 2 &&
        TryResolveTwoPlayerRoundEnd(ctx, m_Players[0].Stocks() == 0, m_Players[1].Stocks() == 0)) {
        return;
    }

    for (const std::size_t playerIndex : blastZonePlayers) {
        RespawnPlayerAtArenaSpawn(playerIndex, kDefaultPlayerHealth);
    }
}

bool InGameState::TryResolveTwoPlayerRoundEnd(AppCtx& ctx,
                                              const bool player1Out,
                                              const bool player2Out) {
    if (player1Out && player2Out) {
        RestartRound();
        return true;
    }

    if (player1Out) {
        StartNextRound(ctx, 1);
        return true;
    }

    if (player2Out) {
        StartNextRound(ctx, 0);
        return true;
    }

    return false;
}

void InGameState::RecordPlayerDefeat(const std::size_t playerIndex, const bool losesStock) {
    if (playerIndex >= m_MatchStats.size()) {
        return;
    }

    ++m_MatchStats[playerIndex].Falls;
    if (losesStock) {
        ++m_MatchStats[playerIndex].StocksLost;
    }
}

void InGameState::RespawnPlayerAtArenaSpawn(const std::size_t playerIndex, const float health) {
    const SDL_FPoint spawnPosition = RespawnPosition(playerIndex, m_Arena);
    const bool facingRight = FacingRightAtSpawn(spawnPosition, m_Arena);
    m_Players[playerIndex].Respawn(spawnPosition, facingRight, health);
}

void InGameState::FinishMatch(AppCtx& ctx, const std::size_t winnerIndex) {
    m_MatchFinished = true;
    m_Paused = false;

    if (m_Players.size() < 2 || m_MatchStats.size() < 2) {
        return;
    }

    ctx.eventDispatcher.Enqueue(NavigationEvent{
        .Action = NavigationAction::ShowMatchResults,
        .Match = m_MatchConfig,
        .Results =
            MatchResults{
                .WinnerIndex = winnerIndex,
                .Player1RoundsWon = m_Players[0].RoundsWon(),
                .Player2RoundsWon = m_Players[1].RoundsWon(),
                .Player1Stats = m_MatchStats[0],
                .Player2Stats = m_MatchStats[1],
            },
    });
}

void InGameState::StartNextRound(AppCtx& ctx, const std::size_t winnerIndex) {
    if (winnerIndex < m_Players.size()) {
        m_Players[winnerIndex].WinRound();

        if (m_Players[winnerIndex].RoundsWon() >= m_MatchConfig.TargetRoundsToWin) {
            FinishMatch(ctx, winnerIndex);
            return;
        }
    }

    ++m_CurrentRound;
    RestartRound();
}

void InGameState::RestartRound() {
    for (std::size_t playerIndex = 0; playerIndex < m_Players.size(); ++playerIndex) {
        const SDL_FPoint spawnPosition = PlayerStartPosition(playerIndex);
        const bool facingRight = FacingRightAtSpawn(spawnPosition, m_Arena);
        m_Players[playerIndex].ResetStocks(m_MatchConfig.StocksPerRound);
        m_Players[playerIndex].Respawn(spawnPosition, facingRight, kDefaultPlayerHealth);
    }
}

}  // namespace sop
