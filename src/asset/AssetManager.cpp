#include "smashorpass/asset/AssetManager.hpp"

#include "smashorpass/core/Base.hpp"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <string_view>
#include <vector>

namespace sop {

	AssetManager::AssetManager(std::filesystem::path assetRootDir, SDL_Renderer *renderer)
		: m_AssetRootDir(std::move(assetRootDir)), m_Renderer(renderer)
	{
		SOP_ASSERT(m_Renderer != nullptr, "Asset manager requires a valid SDL renderer");
	}

	const SpriteSheet &AssetManager::getSpriteSheet(CharacterId character, CharacterAnimation animation)
	{
		auto characterIt = m_SpriteSheets.find(character);
		if (characterIt != m_SpriteSheets.end()) {
			auto animationIt = characterIt->second.find(animation);
			if (animationIt != characterIt->second.end()) {
				return animationIt->second;
			}
		}

		return loadSpriteSheet(character, animation);
	}

	const SpriteSheet &AssetManager::loadSpriteSheet(CharacterId character, CharacterAnimation animation)
	{
		const auto AnimationBaseName = [](CharacterAnimation animationId) -> std::string_view {
			switch (animationId) {
				case CharacterAnimation::Ascending:
					return "Ascending";
				case CharacterAnimation::Attacks:
					return "Attacks";
				case CharacterAnimation::Dash:
					return "Dash";
				case CharacterAnimation::Falling:
					return "Falling";
				case CharacterAnimation::Idle:
					return "Idle";
				case CharacterAnimation::Jump:
					return "Jump";
				case CharacterAnimation::Walk:
					return "Walk";
			}

			SOP_ASSERT(false, "Unhandled character animation");
			return "";
		};

		const auto ReadBytes = [](const std::filesystem::path& path) -> std::vector<uint8_t> {
			std::ifstream file(path, std::ios::binary);
			SOP_ASSERT(file.is_open(), "Failed to open asset file");

			return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
										std::istreambuf_iterator<char>());
		};

		const auto CharacterDirName = [](CharacterId characterId) -> std::string_view {
			switch (characterId) {
				case CharacterId::Robot: {
					return "robot";
				}
				default: {
					SOP_ASSERT(false, "Unhandled character id");
				}
			}
			return "";
		};

		const std::filesystem::path basePath = m_AssetRootDir / "sprites" / "characters" /
											   CharacterDirName(character) /
											   AnimationBaseName(animation);

		const auto spriteBytes		= ReadBytes(basePath.string() + ".png");
		const auto hitboxBytes		= ReadBytes(basePath.string() + "_boxes.png");
		const auto metadataBytes	= ReadBytes(basePath.string() + ".json");

		auto &animations = m_SpriteSheets[character];
		SpriteSheet spriteSheet = SpriteSheet::parse(spriteBytes, hitboxBytes, metadataBytes);
		spriteSheet.createSpriteTexture(m_Renderer);

		auto [it, inserted] = animations.try_emplace(animation, std::move(spriteSheet));

		SOP_ASSERT(inserted, "Sprite sheet should only be loaded once");

		return it->second;
	}
} // namespace sop
