#include "smashorpass/core/SpriteAnimationPlayer.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("sprite animation frame advances modulo the frame count")
{
    sop::SpriteAnimationPlayer player(sop::CharacterAnimation::Idle);

    player.Advance(4);
    REQUIRE(player.GetFrameIndex() == 1);

    player.Advance(4);
    player.Advance(4);
    player.Advance(4);

    REQUIRE(player.GetFrameIndex() == 0);
    REQUIRE(player.GetTickCount() == 4);
}

TEST_CASE("changing animation resets sprite playback to the first frame")
{
    sop::SpriteAnimationPlayer player(sop::CharacterAnimation::Idle);

    player.Advance(6);
    player.Advance(6);

    player.SetAnimation(sop::CharacterAnimation::Walk);

    REQUIRE(player.GetAnimation() == sop::CharacterAnimation::Walk);
    REQUIRE(player.GetFrameIndex() == 0);
    REQUIRE(player.GetTickCount() == 0);
}
