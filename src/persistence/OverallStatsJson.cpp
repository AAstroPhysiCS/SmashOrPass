#include "smashorpass/persistence/OverallStatsJson.hpp"

#include <nlohmann/json.hpp>

namespace sop {
// Teach nlohmann::json how to serialize and deserialize these types.
void to_json(nlohmann::json& json, const PlayerMatchStats& stats) {
    json = nlohmann::json{
        {"damage_dealt", stats.DamageDealt},
        {"damage_taken", stats.DamageTaken},
        {"hits_landed", stats.HitsLanded},
        {"hits_taken", stats.HitsTaken},
        {"head_hits_landed", stats.HeadHitsLanded},
        {"head_hits_taken", stats.HeadHitsTaken},
        {"torso_hits_landed", stats.TorsoHitsLanded},
        {"torso_hits_taken", stats.TorsoHitsTaken},
        {"leg_hits_landed", stats.LegHitsLanded},
        {"leg_hits_taken", stats.LegHitsTaken},
        {"stocks_lost", stats.StocksLost},
        {"falls", stats.Falls},
    };
}

void from_json(const nlohmann::json& json, PlayerMatchStats& stats) {
    stats.DamageDealt = json.value("damage_dealt", 0.0f);
    stats.DamageTaken = json.value("damage_taken", 0.0f);
    stats.HitsLanded = json.value("hits_landed", 0);
    stats.HitsTaken = json.value("hits_taken", 0);
    stats.HeadHitsLanded = json.value("head_hits_landed", 0);
    stats.HeadHitsTaken = json.value("head_hits_taken", 0);
    stats.TorsoHitsLanded = json.value("torso_hits_landed", 0);
    stats.TorsoHitsTaken = json.value("torso_hits_taken", 0);
    stats.LegHitsLanded = json.value("leg_hits_landed", 0);
    stats.LegHitsTaken = json.value("leg_hits_taken", 0);
    stats.StocksLost = json.value("stocks_lost", 0);
    stats.Falls = json.value("falls", 0);
}

void to_json(nlohmann::json& json, const OverallMatchupStats& stats) {
    json = nlohmann::json{
        {"matches_played", stats.MatchesPlayed},
        {"player1_wins", stats.Player1Wins},
        {"opponent_wins", stats.OpponentWins},
        {"player1_totals", stats.Player1Totals},
        {"opponent_totals", stats.OpponentTotals},
    };
}

void from_json(const nlohmann::json& json, OverallMatchupStats& stats) {
    stats.MatchesPlayed = json.value("matches_played", 0);
    stats.Player1Wins = json.value("player1_wins", 0);
    stats.OpponentWins = json.value("opponent_wins", 0);
    stats.Player1Totals = json.value("player1_totals", PlayerMatchStats{});
    stats.OpponentTotals = json.value("opponent_totals", PlayerMatchStats{});
}

}  // namespace sop
