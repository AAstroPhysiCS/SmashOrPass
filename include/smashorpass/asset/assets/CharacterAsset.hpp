#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sop {

using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

struct CharacterSpriteSheetFrame {
    // Where the frame is in the sprite sheet
    SDL_FRect m_Location;
    // Signed local offset from the frames top-left corner to the animation anchor
    SDL_Point m_Anchor;
    SDL_FRect m_CollisionBox;
};

struct CharacterSpriteSheet {
    TexturePtr m_Texture{nullptr, SDL_DestroyTexture};
    std::vector<CharacterSpriteSheetFrame> m_Frames;
};

enum class CharacterAnimation {
    Idle,
    Walk,
    Ascending,
    Falling,
    Attacks,
    Dash,
};

struct CharacterAsset {
    std::string m_Id;
    std::unordered_map<CharacterAnimation, CharacterSpriteSheet> m_SpriteSheets;
};

}  // namespace sop
