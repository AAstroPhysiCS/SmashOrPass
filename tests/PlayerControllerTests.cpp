#include <catch2/catch_test_macros.hpp>

#include "smashorpass/core/PlayerController.hpp"

TEST_CASE("samurai is the default player character for now") {
    const sop::PlayerCharacterState player;

    CHECK(sop::kDefaultCharacterId == sop::CharacterId::Samurai);
    CHECK(player.Character == sop::CharacterId::Samurai);
}
