#pragma once

#include <SDL3/SDL_render.h>

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
    void preloadCharacterSpriteSheets(CharacterId character);

   private:
    [[nodiscard]] const SpriteSheet& loadSpriteSheet(CharacterId character,
                                                     CharacterAnimation animation);

    std::filesystem::path m_AssetRootDir;
    SDL_Renderer* m_Renderer = nullptr;
    std::unordered_map<CharacterId,
                       std::unordered_map<CharacterAnimation, SpriteSheet, EnumClassHash>,
                       EnumClassHash>
        m_SpriteSheets;
};
}  // namespace sop
