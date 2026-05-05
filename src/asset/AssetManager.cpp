#include "smashorpass/asset/AssetManager.hpp"

#include <SDL3_image/SDL_image.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "smashorpass/core/Base.hpp"
#include "spdlog/spdlog.h"

namespace sop {
namespace {

struct SpriteSheetBytes {
    std::vector<uint8_t> Sprite;
    std::vector<uint8_t> Hitbox;
    std::vector<uint8_t> Metadata;
};

[[nodiscard]] std::optional<std::vector<uint8_t>> TryReadBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
}

[[nodiscard]] std::vector<uint8_t> ReadBytes(const std::filesystem::path& path) {
    std::optional<std::vector<uint8_t>> bytes = TryReadBytes(path);
    SOP_VERIFY(bytes.has_value(), "Failed to open asset file");

    return std::move(*bytes);
}

void AppendPath(std::string& paths, const std::filesystem::path& path) {
    if (!paths.empty()) {
        paths += ", ";
    }

    paths += path.string();
}

[[nodiscard]] std::vector<uint8_t> MakeErrorSpriteSheetMetadata() {
    constexpr std::string_view kMetadata = R"json({
  "character": "error",
  "animation": "error",
  "sheet_width": 2048,
  "sheet_height": 2048,
  "frames": [
    {
      "source": "ERROR.png",
      "x_left": 0,
      "x_right": 482,
      "y_top": 0,
      "y_bottom": 482,
      "anchor_x": 241,
      "anchor_y": 241,
      "collision_box": {
        "x": 0,
        "y": 0,
        "width": 482,
        "height": 482
      }
    }
  ]
})json";

    std::vector<uint8_t> bytes;
    bytes.reserve(kMetadata.size());
    for (const char c : kMetadata) {
        bytes.push_back(static_cast<uint8_t>(c));
    }
    return bytes;
}

[[nodiscard]] SpriteSheetBytes ReadCharacterSpriteSheetBytes(
    const std::filesystem::path& assetRootDir,
    const std::filesystem::path& spritePath,
    const std::filesystem::path& hitboxPath,
    const std::filesystem::path& metadataPath) {
    std::optional<std::vector<uint8_t>> spriteBytes = TryReadBytes(spritePath);
    std::optional<std::vector<uint8_t>> hitboxBytes = TryReadBytes(hitboxPath);
    std::optional<std::vector<uint8_t>> metadataBytes = TryReadBytes(metadataPath);

    if (spriteBytes.has_value() && hitboxBytes.has_value() && metadataBytes.has_value()) {
        return SpriteSheetBytes{
            .Sprite = std::move(*spriteBytes),
            .Hitbox = std::move(*hitboxBytes),
            .Metadata = std::move(*metadataBytes),
        };
    }

    std::string missingPaths;
    if (!spriteBytes.has_value()) {
        AppendPath(missingPaths, spritePath);
    }
    if (!hitboxBytes.has_value()) {
        AppendPath(missingPaths, hitboxPath);
    }
    if (!metadataBytes.has_value()) {
        AppendPath(missingPaths, metadataPath);
    }

    const std::filesystem::path fallbackPath = assetRootDir / "sprites" / "ERROR.png";
    spdlog::warn("Missing character sprite sheet asset(s): {}. Using fallback sprite: {}",
                 missingPaths,
                 fallbackPath.string());

    std::vector<uint8_t> fallbackSpriteBytes = ReadBytes(fallbackPath);
    std::vector<uint8_t> fallbackHitboxBytes = fallbackSpriteBytes;

    return SpriteSheetBytes{
        .Sprite = std::move(fallbackSpriteBytes),
        .Hitbox = std::move(fallbackHitboxBytes),
        .Metadata = MakeErrorSpriteSheetMetadata(),
    };
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
            case CharacterId::Samurai: {
                return "samurai";
            }
        }

        SOP_ASSERT(false, "Unhandled character id");
        return "";
    };

    const std::filesystem::path basePath = m_AssetRootDir / "sprites" / "characters" /
                                           CharacterDirName(character) /
                                           AnimationBaseName(animation);

    const SpriteSheetBytes bytes = ReadCharacterSpriteSheetBytes(m_AssetRootDir,
                                                                 basePath.string() + ".png",
                                                                 basePath.string() + "_boxes.png",
                                                                 basePath.string() + ".json");

    auto& animations = m_SpriteSheets[character];
    SpriteSheet spriteSheet = SpriteSheet::parse(bytes.Sprite, bytes.Hitbox, bytes.Metadata);
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
