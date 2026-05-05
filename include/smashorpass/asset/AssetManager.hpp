#pragma once

#include <SDL3/SDL_render.h>

#include <filesystem>
#include <memory>
#include <span>
#include <unordered_map>

#include "smashorpass/asset/ArenaMetadata.hpp"
#include "smashorpass/asset/SpriteSheet.hpp"

namespace sop {

enum class CharacterId {
    Robot,
    Samurai,
};

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
    Jump,
    Walk,
};

struct EnumClassHash {
    template <typename T>
    std::size_t operator()(T value) const {
        return static_cast<std::size_t>(value);
    }
};

class AssetManager {
   public:
    explicit AssetManager(std::filesystem::path assetRootDir, SDL_Renderer* renderer);
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    [[nodiscard]] const SpriteSheet& getSpriteSheet(CharacterId character,
                                                    CharacterAnimation animation);
    [[nodiscard]] SDL_Texture* getArenaBackgroundTexture(ArenaId arena);
    [[nodiscard]] SDL_Texture* getArenaForegroundTexture(ArenaId arena);
    [[nodiscard]] std::span<const SDL_FRect> getArenaCollisionBoxes(ArenaId arena);
    void preloadCharacterSpriteSheets(CharacterId character);

   private:
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* texture) const {
            if (texture != nullptr) {
                SDL_DestroyTexture(texture);
            }
        }
    };

    using TexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

    struct ArenaAsset {
        TexturePtr BackgroundTexture;
        TexturePtr ForegroundTexture;
        ArenaMetadata Metadata;
    };

    [[nodiscard]] const SpriteSheet& loadSpriteSheet(CharacterId character,
                                                     CharacterAnimation animation);
    [[nodiscard]] ArenaAsset& getArenaAsset(ArenaId arena);
    [[nodiscard]] ArenaAsset& loadArenaAsset(ArenaId arena);

    std::filesystem::path m_AssetRootDir;
    SDL_Renderer* m_Renderer = nullptr;
    std::unordered_map<ArenaId, ArenaAsset, EnumClassHash> m_Arenas;
    std::unordered_map<CharacterId,
                       std::unordered_map<CharacterAnimation, SpriteSheet, EnumClassHash>,
                       EnumClassHash>
        m_SpriteSheets;
};
}  // namespace sop
