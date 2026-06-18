#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "smashorpass/asset/assets/CharacterCombatData.hpp"
#include "smashorpass/asset/effects/CharacterFrameEffectMask.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct AppCtx;

using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

struct CharacterSpriteSheetFrame {
    SDL_FRect m_Location;
    SDL_Point m_Anchor;
    SDL_FRect m_CollisionBox;
    HitBox m_HitBox;
    HurtBox m_HurtBox;
};

struct CharacterSpriteSheet {
    TexturePtr m_Texture{nullptr, SDL_DestroyTexture};
    std::vector<CharacterSpriteSheetFrame> m_Frames;

    CharacterFrameEffectMasks::Set m_EffectMasks;

    [[nodiscard]] std::span<const FrameEffectMask> GetEffectMasks(
        CharacterFrameEffectMaskType type) const {
        return m_EffectMasks.Get(type);
    }
};

enum class CharacterAnimation {
    Idle,
    Walk,
    Ascending,
    Falling,
    Attacks,
    Dash,
};

std::string_view CharacterAnimationName(CharacterAnimation animation);

struct CharacterAssetLoadJob;
struct RawCharacterAssetData;

struct CharacterAssetData {
    std::string m_Id;
    std::unordered_map<CharacterAnimation, CharacterSpriteSheet> m_SpriteSheets;

    static CharacterAssetData Default(AppCtx& ctx, const CharacterAssetLoadJob& loadJob);
};

struct CharacterAssetDiscoverer {
    static Result<std::vector<CharacterAssetLoadJob>> ListAvailableAssets(AppCtx& ctx);
};

struct CharacterAssetLoadJob {
    std::string m_Id;

    RawCharacterAssetData ToRawAssetData(AppCtx& ctx);
};

struct RawCharacterSpriteSheet {
    CharacterAnimation m_Animation = CharacterAnimation::Idle;
    SurfacePtr m_Surface{nullptr, SDL_DestroySurface};
    SurfacePtr m_CombatSurface{nullptr, SDL_DestroySurface};
    std::vector<CharacterSpriteSheetFrame> m_Frames;
};

struct RawCharacterAssetData {
    std::string m_Id;
    std::string m_Error;
    std::unordered_map<CharacterAnimation, RawCharacterSpriteSheet> m_SpriteSheets;

    CharacterAssetData ToAssetData(AppCtx& ctx);
};

}  // namespace sop
