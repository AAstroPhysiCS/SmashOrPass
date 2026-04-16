#pragma once

#include <filesystem>
#include <unordered_map>

#include "smashorpass/asset/SpriteSheet.hpp"

namespace sop {
enum class CharacterId {
	Robot,
};

enum class CharacterAnimation {
	Ascending,
	Attacks,
	Dash,
	Falling,
	Idle,
	Jump,
	Walk,
};

struct EnumClassHash {
	template <typename T>
	[[nodiscard]] std::size_t operator()(T value) const noexcept
	{
		return static_cast<std::size_t>(value);
	}
};

class AssetManager {
  public:
	explicit AssetManager(std::filesystem::path assetRootDir);
	AssetManager(const AssetManager &) = delete;
	AssetManager &operator=(const AssetManager &) = delete;
	AssetManager(AssetManager &&) = delete;
	AssetManager &operator=(AssetManager &&) = delete;

	[[nodiscard]] const SpriteSheet &getSpriteSheet(CharacterId character, CharacterAnimation animation);

  private:
	[[nodiscard]] const SpriteSheet &loadSpriteSheet(CharacterId character, CharacterAnimation animation);

	std::filesystem::path m_AssetRootDir;
	std::unordered_map<CharacterId, std::unordered_map<CharacterAnimation, SpriteSheet, EnumClassHash>, EnumClassHash>
	    m_SpriteSheets;
};
} // namespace sop
