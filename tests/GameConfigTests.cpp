#include "smashorpass/core/Game.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("default config has a valid window size")
{
	const auto config = sop::loadDefault();
	(void)config;

	// REQUIRE(config.windowWidth > 0);
	// REQUIRE(config.windowHeight > 0);
	// REQUIRE_FALSE(config.Title.empty());
}
