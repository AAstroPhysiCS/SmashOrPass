#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/core/PlayerSpritePlacement.hpp"

TEST_CASE("player sprite placement keeps the true center anchored for right-facing sprites") {
    const SDL_FRect placeholderRect{10.0f, 20.0f, 116.0f, 192.0f};
    const sop::SpriteSheetFrame frame{
        .x_left = 100,
        .x_right = 300,
        .y_top = 50,
        .y_bottom = 450,
        .source_w = 200,
        .source_h = 400,
        .center_x = 160,
        .center_y = 250,
    };

    const sop::detail::PlayerSpritePlacement placement =
        sop::detail::MakePlayerSpritePlacement(placeholderRect, frame, true, 192.0f);

    REQUIRE(placement.Flip == SDL_FLIP_NONE);
    REQUIRE_THAT(placement.DestinationRect.x, Catch::Matchers::WithinAbs(39.2f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.y, Catch::Matchers::WithinAbs(20.0f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.w, Catch::Matchers::WithinAbs(96.0f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.h, Catch::Matchers::WithinAbs(192.0f, 0.0001f));
    REQUIRE_THAT(placement.Origin.x, Catch::Matchers::WithinAbs(28.8f, 0.0001f));
    REQUIRE_THAT(placement.Origin.y, Catch::Matchers::WithinAbs(96.0f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.x + placement.Origin.x,
                 Catch::Matchers::WithinAbs(68.0f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.y + placement.Origin.y,
                 Catch::Matchers::WithinAbs(116.0f, 0.0001f));
}

TEST_CASE(
    "player sprite placement uses the true center as the flip origin for left-facing sprites") {
    const SDL_FRect placeholderRect{10.0f, 20.0f, 116.0f, 192.0f};
    const sop::SpriteSheetFrame frame{
        .x_left = 100,
        .x_right = 300,
        .y_top = 50,
        .y_bottom = 450,
        .source_w = 200,
        .source_h = 400,
        .center_x = 160,
        .center_y = 250,
    };

    const sop::detail::PlayerSpritePlacement placement =
        sop::detail::MakePlayerSpritePlacement(placeholderRect, frame, false, 192.0f);

    REQUIRE(placement.Flip == SDL_FLIP_HORIZONTAL);
    REQUIRE_THAT(placement.DestinationRect.x + placement.Origin.x,
                 Catch::Matchers::WithinAbs(68.0f, 0.0001f));
    REQUIRE_THAT(placement.DestinationRect.y + placement.Origin.y,
                 Catch::Matchers::WithinAbs(116.0f, 0.0001f));
    REQUIRE_THAT(placement.Origin.x, Catch::Matchers::WithinAbs(28.8f, 0.0001f));
    REQUIRE_THAT(placement.Origin.y, Catch::Matchers::WithinAbs(96.0f, 0.0001f));
}
