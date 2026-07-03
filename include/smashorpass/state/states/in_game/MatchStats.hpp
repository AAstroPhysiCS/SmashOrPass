#pragma once

namespace sop {

struct PlayerMatchStats {
    float DamageDealt = 0.0f;
    float DamageTaken = 0.0f;
    int HitsLanded = 0;
    int HitsTaken = 0;
    int HeadHitsLanded = 0;
    int HeadHitsTaken = 0;
    int TorsoHitsLanded = 0;
    int TorsoHitsTaken = 0;
    int LegHitsLanded = 0;
    int LegHitsTaken = 0;
    int StocksLost = 0;
    int Falls = 0;
};

}  // namespace sop
