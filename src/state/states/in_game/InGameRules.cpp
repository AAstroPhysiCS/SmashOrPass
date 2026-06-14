#include "smashorpass/state/states/in_game/InGameState.hpp"

#include <cstddef>
#include <vector>

#include "smashorpass/state/states/in_game/Defaults.hpp"
#include "smashorpass/util.hpp"

namespace sop {

namespace {

[[nodiscard]] float BottomBlastZone(const Arena& arena) {
    return static_cast<float>(arena.dimensions.y + arena.dimensions.h) + kBottomBlastZonePadding;
}

[[nodiscard]] bool IsBelowBlastZone(const Player& player, const Arena& arena) {
    return player.Position().y > BottomBlastZone(arena);
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

Result<void> InGameState::ResolveDeathsAndRespawns() {
    std::vector<std::size_t> deadPlayers;
    for (std::size_t playerIndex = 0; playerIndex < m_Players.size(); ++playerIndex) {
        if (IsBelowBlastZone(m_Players[playerIndex], m_Arena)) {
            deadPlayers.push_back(playerIndex);
        }
    }

    if (deadPlayers.empty()) {
        return Ok();
    }

    for (const std::size_t playerIndex : deadPlayers) {
        m_Players[playerIndex].LoseStock();
    }

    if (m_Players.size() == 2) {
        const bool player1Out = m_Players[0].Stocks() == 0;
        const bool player2Out = m_Players[1].Stocks() == 0;

        if (player1Out && player2Out) {
            RestartRound();
            return Ok();
        }

        if (player1Out) {
            StartNextRound(1);
            return Ok();
        }

        if (player2Out) {
            StartNextRound(0);
            return Ok();
        }
    }

    for (const std::size_t playerIndex : deadPlayers) {
        const SDL_FPoint spawnPosition = RespawnPosition(playerIndex, m_Arena);
        const bool facingRight = FacingRightAtSpawn(spawnPosition, m_Arena);
        m_Players[playerIndex].Respawn(spawnPosition, facingRight, kDefaultPlayerHealth);
    }

    return Ok();
}

void InGameState::StartNextRound(const std::size_t winnerIndex) {
    if (winnerIndex < m_Players.size()) {
        m_Players[winnerIndex].WinRound();
    }

    ++m_CurrentRound;
    RestartRound();
}

void InGameState::RestartRound() {
    for (std::size_t playerIndex = 0; playerIndex < m_Players.size(); ++playerIndex) {
        const SDL_FPoint spawnPosition = PlayerStartPosition(playerIndex);
        const bool facingRight = FacingRightAtSpawn(spawnPosition, m_Arena);
        m_Players[playerIndex].ResetStocks(kDefaultPlayerStocks);
        m_Players[playerIndex].Respawn(spawnPosition, facingRight, kDefaultPlayerHealth);
    }
}

}  // namespace sop
