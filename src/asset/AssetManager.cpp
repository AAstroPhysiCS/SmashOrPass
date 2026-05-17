#include "smashorpass/asset/AssetManager.hpp"

#include <SDL3_image/SDL_image.h>

#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "SDL3/SDL.h"
#include "smashorpass/core/Base.hpp"
#include "spdlog/spdlog.h"

namespace sop {

using namespace sop_util;

namespace {

struct SpriteSheetBytes {
    std::vector<uint8_t> Sprite;
    std::vector<uint8_t> Hitbox;
    std::vector<uint8_t> Metadata;
};

Result<std::vector<uint8_t>> ReadBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Err(std::format("Failed to open asset file: {}", path.string()));
    }

    std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(file),
                               std::istreambuf_iterator<char>()};
    if (file.bad()) {
        return Err(std::format("Failed to read asset file: {}", path.string()));
    }

    return Ok(std::move(bytes));
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

Result<SpriteSheetBytes> ReadCharacterSpriteSheetBytes(
    const std::filesystem::path& assetRootDir,
    const std::filesystem::path& spritePath,
    const std::filesystem::path& hitboxPath,
    const std::filesystem::path& metadataPath) {
    Result<std::vector<uint8_t>> spriteBytes = ReadBytes(spritePath);
    Result<std::vector<uint8_t>> hitboxBytes = ReadBytes(hitboxPath);
    Result<std::vector<uint8_t>> metadataBytes = ReadBytes(metadataPath);

    if (spriteBytes.has_value() && hitboxBytes.has_value() && metadataBytes.has_value()) {
        return Ok(SpriteSheetBytes{
            .Sprite = std::move(*spriteBytes),
            .Hitbox = std::move(*hitboxBytes),
            .Metadata = std::move(*metadataBytes),
        });
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

    TRY(fallbackSpriteBytes, ReadBytes(fallbackPath));
    std::vector<uint8_t> fallbackHitboxBytes = fallbackSpriteBytes;

    return Ok(SpriteSheetBytes{
        .Sprite = std::move(fallbackSpriteBytes),
        .Hitbox = std::move(fallbackHitboxBytes),
        .Metadata = MakeErrorSpriteSheetMetadata(),
    });
}

Result<std::string_view> ArenaBaseName(ArenaId arenaId) {
    switch (arenaId) {
        case ArenaId::Chains:
            return Ok(std::string_view{"chains"});
    }

    return Err(std::string("Unhandled arena id"));
}

}  // namespace

Result<std::unique_ptr<AssetManager>> AssetManager::Create(std::filesystem::path assetRootDir,
                                                           SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        return Err(std::string("AssetManager::Create failed: renderer is null"));
    }

    return Ok(std::unique_ptr<AssetManager>(new AssetManager(std::move(assetRootDir), renderer)));
}

AssetManager::AssetManager(std::filesystem::path assetRootDir, SDL_Renderer* renderer)
    : m_AssetRootDir(std::move(assetRootDir)), m_Renderer(renderer) {}

Result<std::reference_wrapper<const SpriteSheet>> AssetManager::getSpriteSheet(
    CharacterId character, CharacterAnimation animation) {
    auto characterIt = m_SpriteSheets.find(character);
    if (characterIt != m_SpriteSheets.end()) {
        auto animationIt = characterIt->second.find(animation);
        if (animationIt != characterIt->second.end()) {
            return Ok(std::cref(animationIt->second));
        }
    }

    return loadSpriteSheet(character, animation);
}

Result<SDL_Texture*> AssetManager::getArenaBackgroundTexture(ArenaId arena) {
    TRY(asset, getArenaAsset(arena));
    SDL_Texture* texture = asset.get().BackgroundTexture.get();
    if (texture == nullptr) {
        return Err(std::string("Arena background texture is null"));
    }
    return Ok(texture);
}

Result<SDL_Texture*> AssetManager::getArenaForegroundTexture(ArenaId arena) {
    TRY(asset, getArenaAsset(arena));
    SDL_Texture* texture = asset.get().ForegroundTexture.get();
    if (texture == nullptr) {
        return Err(std::string("Arena foreground texture is null"));
    }
    return Ok(texture);
}

Result<std::span<const SDL_FRect>> AssetManager::getArenaCollisionBoxes(ArenaId arena) {
    TRY(asset, getArenaAsset(arena));
    return Ok(asset.get().Metadata.getCollisionBoxes());
}

Result<void> AssetManager::preloadCharacterSpriteSheets(CharacterId character) {
    constexpr std::array kAnimations{
        CharacterAnimation::Ascending,
        CharacterAnimation::Attacks,
        CharacterAnimation::Dash,
        CharacterAnimation::Falling,
        CharacterAnimation::Idle,
        CharacterAnimation::Walk,
    };

    for (const CharacterAnimation animation : kAnimations) {
        TRY(spriteSheet, getSpriteSheet(character, animation));
        (void)spriteSheet;
    }
    return Ok();
}

Result<std::span<const FrameEffectMask>> AssetManager::GetCharacterAnimationEffectMasks(
    CharacterId character, CharacterAnimation animation, EffectMaskKind kind) {
    const CharacterAnimationEffectMaskKey key{
        .Character = character,
        .Animation = animation,
        .Kind = kind,
    };

    const auto it = m_CharacterAnimationEffectMasks.find(key);
    if (it != m_CharacterAnimationEffectMasks.end()) {
        return Ok(std::span<const FrameEffectMask>{it->second.data(), it->second.size()});
    }

    TRY(masksRef, LoadCharacterAnimationEffectMasks(character, animation, kind));
    const std::vector<FrameEffectMask>& masks = masksRef.get();

    return Ok(std::span<const FrameEffectMask>{masks.data(), masks.size()});
}

Result<std::reference_wrapper<const SpriteSheet>> AssetManager::loadSpriteSheet(
    CharacterId character, CharacterAnimation animation) {
    TRY(characterDir, GetCharacterDirName(character));
    TRY(animationBase, GetAnimationBaseName(animation));
    const std::filesystem::path basePath = m_AssetRootDir / "sprites" / "characters" /
                                           std::string{characterDir} / std::string{animationBase};

    TRY(bytes,
        ReadCharacterSpriteSheetBytes(m_AssetRootDir,
                                      basePath.string() + ".png",
                                      basePath.string() + "_boxes.png",
                                      basePath.string() + ".json"));

    auto& animations = m_SpriteSheets[character];
    TRY(spriteSheet, SpriteSheet::parse(bytes.Sprite, bytes.Hitbox, bytes.Metadata));
    TRY_VOID(spriteSheet.createSpriteTexture(m_Renderer));

    auto [it, inserted] = animations.try_emplace(animation, std::move(spriteSheet));

    if (!inserted) {
        return Err(std::string("Sprite sheet should only be loaded once"));
    }

    return Ok(std::cref(it->second));
}

Result<std::reference_wrapper<AssetManager::ArenaAsset>> AssetManager::getArenaAsset(
    ArenaId arena) {
    auto it = m_Arenas.find(arena);
    if (it != m_Arenas.end()) {
        return Ok(std::ref(it->second));
    }

    return loadArenaAsset(arena);
}

Result<std::reference_wrapper<AssetManager::ArenaAsset>> AssetManager::loadArenaAsset(
    ArenaId arena) {
    TRY(arenaBaseName, ArenaBaseName(arena));
    const std::filesystem::path basePath =
        m_AssetRootDir / "sprites" / "arenas" / std::string{arenaBaseName};
    const std::filesystem::path backgroundTexturePath = basePath.string() + "_background.png";
    const std::filesystem::path foregroundTexturePath = basePath.string() + "_foreground.png";
    const std::filesystem::path metadataPath = basePath.string() + ".json";
    const std::string backgroundTexturePathString = backgroundTexturePath.string();
    const std::string foregroundTexturePathString = foregroundTexturePath.string();
    TRY(metadataBytes, ReadBytes(metadataPath));

    TexturePtr backgroundTexture{IMG_LoadTexture(m_Renderer, backgroundTexturePathString.c_str())};
    if (backgroundTexture == nullptr) {
        return Err(std::format("Failed to load arena background texture '{}': {}",
                               backgroundTexturePathString,
                               SDL_GetError()));
    }

    TexturePtr foregroundTexture{IMG_LoadTexture(m_Renderer, foregroundTexturePathString.c_str())};
    if (foregroundTexture == nullptr) {
        return Err(std::format("Failed to load arena foreground texture '{}': {}",
                               foregroundTexturePathString,
                               SDL_GetError()));
    }

    TRY(metadata, ArenaMetadata::parse(metadataBytes));

    ArenaAsset arenaAsset{
        .BackgroundTexture = std::move(backgroundTexture),
        .ForegroundTexture = std::move(foregroundTexture),
        .Metadata = std::move(metadata),
    };

    auto [it, inserted] = m_Arenas.try_emplace(arena, std::move(arenaAsset));
    if (!inserted) {
        return Err(std::string("Arena asset should only be loaded once"));
    }

    return Ok(std::ref(it->second));
}

Result<std::string_view> AssetManager::GetCharacterDirName(CharacterId character) const {
    switch (character) {
        case CharacterId::Samurai:
            return Ok(std::string_view{"samurai"});
        case CharacterId::Brawler:
            return Ok(std::string_view{"brawler"});
        case CharacterId::Tank:
            return Ok(std::string_view{"tank"});
        case CharacterId::Mage:
            return Ok(std::string_view{"mage"});
    }
    return Err(std::string("Unhandled character id"));
}

Result<std::string_view> AssetManager::GetAnimationBaseName(CharacterAnimation animation) const {
    switch (animation) {
        case CharacterAnimation::Ascending:
            return Ok(std::string_view{"Ascending"});
        case CharacterAnimation::Attacks:
            return Ok(std::string_view{"Attacks"});
        case CharacterAnimation::Dash:
            return Ok(std::string_view{"Dash"});
        case CharacterAnimation::Falling:
            return Ok(std::string_view{"Falling"});
        case CharacterAnimation::Idle:
            return Ok(std::string_view{"Idle"});
        case CharacterAnimation::Walk:
            return Ok(std::string_view{"Walk"});
    }

    return Err(std::string("Unhandled character animation"));
}

Result<std::reference_wrapper<const std::vector<FrameEffectMask>>>
AssetManager::LoadCharacterAnimationEffectMasks(CharacterId character,
                                                CharacterAnimation animation,
                                                EffectMaskKind kind) {
    const auto CharacterAnimationBasePath =
        [&](const std::filesystem::path& assetRootDir,
            CharacterId character,
            CharacterAnimation animation) -> Result<std::filesystem::path> {
        TRY(characterDir, GetCharacterDirName(character));
        TRY(animationBase, GetAnimationBaseName(animation));
        return Ok(assetRootDir / "sprites" / "characters" / std::string{characterDir} /
                  std::string{animationBase});
    };

    const auto LoadSurfaceFromBytes = [](std::span<const uint8_t> bytes,
                                         const char* name) -> Result<SDL_Surface*> {
        SDL_IOStream* io = SDL_IOFromConstMem(bytes.data(), bytes.size());
        if (io == nullptr) {
            return Err(std::format("Failed to create IO stream for {}: {}", name, SDL_GetError()));
        }

        SDL_Surface* surface = IMG_LoadPNG_IO(io);
        const std::string loadError = surface == nullptr ? SDL_GetError() : "";
        if (!SDL_CloseIO(io)) {
            if (surface != nullptr) {
                SDL_DestroySurface(surface);
            }
            return Err(std::format("SDL_CloseIO failed: {}", SDL_GetError()));
        }
        if (surface == nullptr) {
            return Err(std::format("Failed to load PNG surface for {}: {}", name, loadError));
        }

        return Ok(surface);
    };

    const auto CreateEffectMaskDefinition =
        [](EffectMaskKind kind) -> Result<EffectMaskDefinition> {
        switch (kind) {
            case EffectMaskKind::SwordGreen:
                return Ok(EffectMaskDefinition{
                    .SampleStep = 4,
                    .PixelPredicate =
                        [](const EffectMaskPixel& pixel) {
                            const float r = static_cast<float>(pixel.R);
                            const float g = static_cast<float>(pixel.G);
                            const float b = static_cast<float>(pixel.B);

                            return pixel.A > 64 && g > 110.0f && g > r * 1.25f && g > b * 1.25f;
                        },
                });
        }
        return Err(std::string("Unhandled effect mask kind"));
    };

    TRY(spriteSheetRef, getSpriteSheet(character, animation));
    const SpriteSheet& spriteSheet = spriteSheetRef.get();
    const std::span<const SpriteSheetFrame> frames = spriteSheet.getFrames();

    TRY(basePath, CharacterAnimationBasePath(m_AssetRootDir, character, animation));

    TRY(bytes,
        ReadCharacterSpriteSheetBytes(m_AssetRootDir,
                                      basePath.string() + ".png",
                                      basePath.string() + "_boxes.png",
                                      basePath.string() + ".json"));

    TRY(rawSpriteSurface, LoadSurfaceFromBytes(bytes.Sprite, "effect mask sprite surface"));
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> spriteSurface{rawSpriteSurface};

    TRY(definition, CreateEffectMaskDefinition(kind));

    TRY(masks, BuildEffectMasks(spriteSurface.get(), frames, definition));

    const CharacterAnimationEffectMaskKey key{
        .Character = character,
        .Animation = animation,
        .Kind = kind,
    };

    auto [it, inserted] = m_CharacterAnimationEffectMasks.try_emplace(key, std::move(masks));

    if (!inserted) {
        return Err(std::string("Effect mask should only be loaded once"));
    }

    return Ok(std::cref(it->second));
}

Result<std::vector<FrameEffectMask>> AssetManager::BuildEffectMasks(
    SDL_Surface* surface,
    std::span<const SpriteSheetFrame> frames,
    const EffectMaskDefinition& definition) {
    if (surface == nullptr) {
        return Err(std::string("Effect mask generation requires a valid surface"));
    }
    if (definition.SampleStep <= 0) {
        return Err(std::string("Effect mask sample step must be positive"));
    }
    if (definition.PixelPredicate == nullptr) {
        return Err(std::string("Effect mask requires a pixel predicate"));
    }

    std::vector<FrameEffectMask> masks;
    masks.resize(frames.size());

    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> rgbaSurface{
        SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32)};
    if (rgbaSurface == nullptr) {
        return Err(std::format("SDL_ConvertSurface failed: {}", SDL_GetError()));
    }

    for (const SpriteSheetFrame& frame : frames) {
        if (frame.x_right > static_cast<uint32_t>(rgbaSurface->w) ||
            frame.y_bottom > static_cast<uint32_t>(rgbaSurface->h)) {
            return Err(std::string("Effect mask frame is outside the sprite surface"));
        }
    }

    const bool mustLock = SDL_MUSTLOCK(rgbaSurface.get());
    if (mustLock) {
        if (!SDL_LockSurface(rgbaSurface.get())) {
            return Err(std::format("SDL_LockSurface failed: {}", SDL_GetError()));
        }
    }

    const auto* pixels = static_cast<const Uint8*>(rgbaSurface->pixels);

    for (size_t frameIndex = 0; frameIndex < frames.size(); ++frameIndex) {
        const SpriteSheetFrame& frame = frames[frameIndex];
        FrameEffectMask& mask = masks[frameIndex];

        for (uint32_t y = frame.y_top; y < frame.y_bottom; y += definition.SampleStep) {
            for (uint32_t x = frame.x_left; x < frame.x_right; x += definition.SampleStep) {
                const Uint8* p = pixels + y * rgbaSurface->pitch + x * 4;

                const EffectMaskPixel pixel{
                    .R = p[0],
                    .G = p[1],
                    .B = p[2],
                    .A = p[3],

                    .SheetX = x,
                    .SheetY = y,

                    .LocalX = x - frame.x_left,
                    .LocalY = y - frame.y_top,

                    .FrameIndex = frameIndex,
                };

                if (!definition.PixelPredicate(pixel)) {
                    continue;
                }

                mask.Points.push_back(Vec2{
                    static_cast<float>(pixel.LocalX),
                    static_cast<float>(pixel.LocalY),
                });
            }
        }
    }

    if (mustLock) {
        SDL_UnlockSurface(rgbaSurface.get());
    }

    return Ok(std::move(masks));
}

}  // namespace sop
