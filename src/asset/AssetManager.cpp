#include "smashorpass/asset/AssetManager.hpp"

#include <SDL3_image/SDL_image.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "smashorpass/core/Base.hpp"

namespace sop {
namespace {

[[nodiscard]] std::vector<uint8_t> ReadBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    SOP_ASSERT(file.is_open(), "Failed to open asset file");

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
}

[[nodiscard]] std::string_view ArenaBaseName(ArenaId arenaId) {
    switch (arenaId) {
        case ArenaId::Chains:
            return "chains";
    }

    SOP_ASSERT(false, "Unhandled arena id");
    return "";
}

}  // namespace

AssetManager::AssetManager(std::filesystem::path assetRootDir, SDL_Renderer* renderer)
    : m_AssetRootDir(std::move(assetRootDir)), m_Renderer(renderer) {
    SOP_ASSERT(m_Renderer != nullptr, "Asset manager requires a valid SDL renderer");
}

const SpriteSheet& AssetManager::getSpriteSheet(CharacterId character,
                                                CharacterAnimation animation) {
    auto characterIt = m_SpriteSheets.find(character);
    if (characterIt != m_SpriteSheets.end()) {
        auto animationIt = characterIt->second.find(animation);
        if (animationIt != characterIt->second.end()) {
            return animationIt->second;
        }
    }

    return loadSpriteSheet(character, animation);
}

SDL_Texture* AssetManager::getArenaBackgroundTexture(ArenaId arena) {
    return getArenaAsset(arena).BackgroundTexture.get();
}

SDL_Texture* AssetManager::getArenaForegroundTexture(ArenaId arena) {
    return getArenaAsset(arena).ForegroundTexture.get();
}

std::span<const SDL_FRect> AssetManager::getArenaCollisionBoxes(ArenaId arena) {
    return getArenaAsset(arena).Metadata.getCollisionBoxes();
}

void AssetManager::preloadCharacterSpriteSheets(CharacterId character) {
    constexpr std::array kAnimations{
        CharacterAnimation::Ascending,
        CharacterAnimation::Attacks,
        CharacterAnimation::Dash,
        CharacterAnimation::Falling,
        CharacterAnimation::Idle,
        CharacterAnimation::Jump,
        CharacterAnimation::Walk,
    };

    for (const CharacterAnimation animation : kAnimations) {
        (void)getSpriteSheet(character, animation);
    }
}

const SpriteSheet& AssetManager::loadSpriteSheet(CharacterId character,
                                                 CharacterAnimation animation) {
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

    const auto spriteBytes = ReadBytes(basePath.string() + ".png");
    const auto hitboxBytes = ReadBytes(basePath.string() + "_boxes.png");
    const auto metadataBytes = ReadBytes(basePath.string() + ".json");

    auto& animations = m_SpriteSheets[character];
    SpriteSheet spriteSheet = SpriteSheet::parse(spriteBytes, hitboxBytes, metadataBytes);
    spriteSheet.createSpriteTexture(m_Renderer);

    auto [it, inserted] = animations.try_emplace(animation, std::move(spriteSheet));

    SOP_ASSERT(inserted, "Sprite sheet should only be loaded once");

    return it->second;
}

AssetManager::ArenaAsset& AssetManager::getArenaAsset(ArenaId arena) {
    auto it = m_Arenas.find(arena);
    if (it != m_Arenas.end()) {
        return it->second;
    }

    return loadArenaAsset(arena);
}

AssetManager::ArenaAsset& AssetManager::loadArenaAsset(ArenaId arena) {
    const std::filesystem::path basePath =
        m_AssetRootDir / "sprites" / "arenas" / std::string{ArenaBaseName(arena)};
    const std::filesystem::path backgroundTexturePath = basePath.string() + "_background.png";
    const std::filesystem::path foregroundTexturePath = basePath.string() + "_foreground.png";
    const std::filesystem::path metadataPath = basePath.string() + ".json";
    const std::string backgroundTexturePathString = backgroundTexturePath.string();
    const std::string foregroundTexturePathString = foregroundTexturePath.string();
    const auto metadataBytes = ReadBytes(metadataPath);

    SDL_Texture* rawBackgroundTexture =
        IMG_LoadTexture(m_Renderer, backgroundTexturePathString.c_str());
    SOP_SDL_ASSERT(rawBackgroundTexture != nullptr, "Failed to load arena background texture");

    SDL_Texture* rawForegroundTexture =
        IMG_LoadTexture(m_Renderer, foregroundTexturePathString.c_str());
    SOP_SDL_ASSERT(rawForegroundTexture != nullptr, "Failed to load arena foreground texture");

    ArenaAsset arenaAsset{
        .BackgroundTexture = TexturePtr{rawBackgroundTexture},
        .ForegroundTexture = TexturePtr{rawForegroundTexture},
        .Metadata = ArenaMetadata::parse(metadataBytes),
    };

    auto [it, inserted] = m_Arenas.try_emplace(arena, std::move(arenaAsset));
    SOP_ASSERT(inserted, "Arena asset should only be loaded once");

    return it->second;
}
}  // namespace sop
