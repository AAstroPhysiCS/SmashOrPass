#pragma once

#include <array>
#include <cstddef>

#include "smashorpass/state/states/in_game/MatchConfig.hpp"

namespace sop {

enum class MatchupType {
    Player1VsPlayer2,
    Player1VsAi,
};

struct OverallMatchupStats {
    int MatchesPlayed = 0;
    int Player1Wins = 0;
    int OpponentWins = 0;
    PlayerMatchStats Player1Totals{};
    PlayerMatchStats OpponentTotals{};

    [[nodiscard]] float Player1WinRate() const {
        if (MatchesPlayed == 0) {
            return 0.0f;
        }

        return static_cast<float>(Player1Wins) / static_cast<float>(MatchesPlayed);
    }
};

class OverallStatsTracker {
   public:
    void RecordMatch(const MatchConfig& config, const MatchResults& results) {
        auto& stats = StatsFor(MatchupFor(config));
        ++stats.MatchesPlayed;

        if (results.WinnerIndex == 0) {
            ++stats.Player1Wins;
        } else {
            ++stats.OpponentWins;
        }

        AddStats(stats.Player1Totals, results.Player1Stats);
        AddStats(stats.OpponentTotals, results.Player2Stats);
    }

    [[nodiscard]] const OverallMatchupStats& StatsFor(const MatchConfig& config) const {
        return StatsFor(MatchupFor(config));
    }

    [[nodiscard]] const OverallMatchupStats& StatsFor(const MatchupType matchup) const {
        return m_Stats[ToIndex(matchup)];
    }

    [[nodiscard]] OverallMatchupStats& StatsFor(const MatchupType matchup) {
        return m_Stats[ToIndex(matchup)];
    }

    [[nodiscard]] static MatchupType MatchupFor(const MatchConfig& config) {
        const bool player2IsAi = config.PlayerControls.size() > 1 &&
                                 config.PlayerControls[1] == PlayerControl::Agent;
        return player2IsAi ? MatchupType::Player1VsAi : MatchupType::Player1VsPlayer2;
    }

    [[nodiscard]] static const char* MatchupName(const MatchupType matchup) {
        switch (matchup) {
            case MatchupType::Player1VsPlayer2:
                return "Overall P1 vs P2";
            case MatchupType::Player1VsAi:
                return "Overall P1 vs AI";
        }

        return "Overall";
    }

    [[nodiscard]] static const char* OpponentName(const MatchupType matchup) {
        switch (matchup) {
            case MatchupType::Player1VsPlayer2:
                return "P2";
            case MatchupType::Player1VsAi:
                return "AI";
        }

        return "Opponent";
    }

   private:
    [[nodiscard]] static std::size_t ToIndex(const MatchupType matchup) {
        return static_cast<std::size_t>(matchup);
    }

    static void AddStats(PlayerMatchStats& total, const PlayerMatchStats& match) {
        total.DamageDealt += match.DamageDealt;
        total.DamageTaken += match.DamageTaken;
        total.HitsLanded += match.HitsLanded;
        total.HitsTaken += match.HitsTaken;
        total.HeadHitsLanded += match.HeadHitsLanded;
        total.HeadHitsTaken += match.HeadHitsTaken;
        total.TorsoHitsLanded += match.TorsoHitsLanded;
        total.TorsoHitsTaken += match.TorsoHitsTaken;
        total.LegHitsLanded += match.LegHitsLanded;
        total.LegHitsTaken += match.LegHitsTaken;
        total.StocksLost += match.StocksLost;
        total.Falls += match.Falls;
    }

    std::array<OverallMatchupStats, 2> m_Stats{};
};

}  // namespace sop
