#pragma once

#include <SDL3/SDL_render.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <unordered_map>

#include "smashorpass/asset/ArenaMetadata.hpp"
#include "smashorpass/asset/SpriteSheet.hpp"
#include "smashorpass/core/Base.hpp"
#include "smashorpass/util.hpp"

namespace sop {

using namespace sop_util;

enum class CharacterId { Samurai, Brawler, Tank, Mage };

inline constexpr CharacterId kDefaultCharacterId = CharacterId::Samurai;

enum class ArenaId {
    Chains,
};

enum class CharacterAnimation {
    Ascending,
    Attacks,
    Dash,
    Falling,
    Idle,
    Walk,
};

struct EffectMaskPixel {
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
    uint8_t A = 0;

    uint32_t SheetX = 0;
    uint32_t SheetY = 0;

    uint32_t LocalX = 0;
    uint32_t LocalY = 0;

    size_t FrameIndex = 0;
};

struct EffectMaskDefinition {
    int32_t SampleStep = 4;
    std::function<bool(const EffectMaskPixel&)> PixelPredicate;
};

enum class EffectMaskKind {
    SwordGreen,
};

struct FrameEffectMask {
    std::vector<Vec2> Points;
};

struct CharacterAnimationEffectMaskKey {
    CharacterId Character = CharacterId::Samurai;
    CharacterAnimation Animation = CharacterAnimation::Idle;
    EffectMaskKind Kind = EffectMaskKind::SwordGreen;

    bool operator==(const CharacterAnimationEffectMaskKey& other) const {
        return Character == other.Character && Animation == other.Animation && Kind == other.Kind;
    }
};

struct CharacterAnimationEffectMaskKeyHash {
    std::size_t operator()(const CharacterAnimationEffectMaskKey& key) const {
        const std::size_t character = static_cast<std::size_t>(key.Character);
        const std::size_t animation = static_cast<std::size_t>(key.Animation);
        const std::size_t kind = static_cast<std::size_t>(key.Kind);

        return character ^ (animation << 8U) ^ (kind << 16U);
    }
};

struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T value) const {
        return static_cast<std::size_t>(value);
    }
};

class AssetManager {
   public:
    [[nodiscard]] static Result<std::unique_ptr<AssetManager>> Create(
        std::filesystem::path assetRootDir, SDL_Renderer* renderer);
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    [[nodiscard]] Result<std::reference_wrapper<const SpriteSheet>> getSpriteSheet(
        CharacterId character, CharacterAnimation animation);
    [[nodiscard]] Result<SDL_Texture*> getArenaBackgroundTexture(ArenaId arena);
    [[nodiscard]] Result<SDL_Texture*> getArenaForegroundTexture(ArenaId arena);
    [[nodiscard]] Result<std::span<const SDL_FRect>> getArenaCollisionBoxes(ArenaId arena);
    [[nodiscard]] Result<void> preloadCharacterSpriteSheets(CharacterId character);

    [[nodiscard]] Result<std::span<const FrameEffectMask>> GetCharacterAnimationEffectMasks(
        CharacterId character, CharacterAnimation animation, EffectMaskKind kind);

   private:
    explicit AssetManager(std::filesystem::path assetRootDir, SDL_Renderer* renderer);

    struct SdlTextureDeleter {
        void operator()(SDL_Texture* texture) const {
            if (texture != nullptr) {
                SDL_DestroyTexture(texture);
            }
        }
    };

    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* surface) const {
            if (surface != nullptr) {
                SDL_DestroySurface(surface);
            }
        }
    };

    using TexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

    struct ArenaAsset {
        TexturePtr BackgroundTexture;
        TexturePtr ForegroundTexture;
        ArenaMetadata Metadata;
    };

    [[nodiscard]] Result<std::reference_wrapper<const SpriteSheet>> loadSpriteSheet(
        CharacterId character, CharacterAnimation animation);
    [[nodiscard]] Result<std::reference_wrapper<ArenaAsset>> getArenaAsset(ArenaId arena);
    [[nodiscard]] Result<std::reference_wrapper<ArenaAsset>> loadArenaAsset(ArenaId arena);

    Result<std::string_view> GetCharacterDirName(CharacterId character) const;
    Result<std::string_view> GetAnimationBaseName(CharacterAnimation animation) const;

    [[nodiscard]] Result<std::reference_wrapper<const std::vector<FrameEffectMask>>>
    LoadCharacterAnimationEffectMasks(CharacterId character,
                                      CharacterAnimation animation,
                                      EffectMaskKind kind);
    [[nodiscard]] Result<std::vector<FrameEffectMask>> BuildEffectMasks(
        SDL_Surface* surface,
        std::span<const SpriteSheetFrame> frames,
        const EffectMaskDefinition& definition);

    std::filesystem::path m_AssetRootDir;
    SDL_Renderer* m_Renderer = nullptr;
    std::unordered_map<ArenaId, ArenaAsset, EnumClassHash> m_Arenas;
    std::unordered_map<CharacterId,
                       std::unordered_map<CharacterAnimation, SpriteSheet, EnumClassHash>,
                       EnumClassHash>
        m_SpriteSheets;
    std::unordered_map<CharacterAnimationEffectMaskKey,
                       std::vector<FrameEffectMask>,
                       CharacterAnimationEffectMaskKeyHash>
        m_CharacterAnimationEffectMasks;
};
}  // namespace sop
