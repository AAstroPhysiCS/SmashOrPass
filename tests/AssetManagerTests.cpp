#include "smashorpass/asset/AssetManager.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

TEST_CASE("asset manager loads sprite sheets lazily")
{
	const auto missingAssetsDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "assets-does-not-exist";

	REQUIRE_FALSE(std::filesystem::exists(missingAssetsDir));

	REQUIRE_NOTHROW((void)sop::AssetManager(missingAssetsDir));
}

TEST_CASE("asset manager loads robot dash spritesheet")
{
	const auto assetsDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "assets";

	sop::AssetManager assetManager(assetsDir);

	REQUIRE_NOTHROW((void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Dash));
}

TEST_CASE("asset manager keeps spritesheet references stable across inserts")
{
	const auto assetsDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "assets";

	sop::AssetManager assetManager(assetsDir);

	const sop::SpriteSheet &dash = assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Dash);

	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Ascending);
	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Attacks);
	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Falling);
	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Idle);
	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Jump);
	(void)assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Walk);

	REQUIRE(&dash == &assetManager.getSpriteSheet(sop::CharacterId::Robot, sop::CharacterAnimation::Dash));
}
