#include "smashorpass/asset/SpriteSheet.hpp"

#include <SDL3/SDL_iostream.h>
#include <SDL3_image/SDL_image.h>

#include <nlohmann/json.hpp>

#include "smashorpass/core/Base.hpp"

namespace sop {
SpriteSheet SpriteSheet::parse(std::span<const uint8_t> spriteSheet,
                               std::span<const uint8_t> hitboxSheet,
                               std::span<const uint8_t> metadata) {
    const auto LoadSurfaceFromBytes = [](std::span<const uint8_t> bytes, const char* name) {
        SDL_IOStream* io = SDL_IOFromConstMem(bytes.data(), bytes.size());
        SOP_ASSERT(io != nullptr, "Failed to create IO stream");

        SDL_Surface* surface = IMG_LoadPNG_IO(io);
        SOP_SDL_ASSERT(SDL_CloseIO(io), "SDL_CloseIO");
        SOP_SDL_ASSERT(surface != nullptr, name);

        return surface;
    };

    const auto getU32 = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<uint32_t>();
    };
    const auto getI32 = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<int32_t>();
    };
    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };

    SpriteSheet result;

    result.m_SpriteSurface = LoadSurfaceFromBytes(spriteSheet, "sprite sheet");
    result.m_HitboxSheet = LoadSurfaceFromBytes(hitboxSheet, "hitbox sheet");

    const auto json = nlohmann::json::parse(metadata.begin(), metadata.end());
    const auto& framesJson = json.at("frames");

    result.m_Frames.reserve(framesJson.size());

    for (const auto& frameJson : framesJson) {
        const auto& collisionBoxJson = frameJson.at("collision_box");
        const SDL_FRect collisionBox{
            .x = getFloat(collisionBoxJson, "x"),
            .y = getFloat(collisionBoxJson, "y"),
            .w = getFloat(collisionBoxJson, "width"),
            .h = getFloat(collisionBoxJson, "height"),
        };
        SOP_ASSERT(collisionBox.w >= 0.0f && collisionBox.h >= 0.0f,
                   "Sprite collision box cannot be negative");

        result.m_Frames.push_back(SpriteSheetFrame{
            .x_left = getU32(frameJson, "x_left"),
            .x_right = getU32(frameJson, "x_right"),
            .y_top = getU32(frameJson, "y_top"),
            .y_bottom = getU32(frameJson, "y_bottom"),
            .anchor_x = getI32(frameJson, "anchor_x"),
            .anchor_y = getI32(frameJson, "anchor_y"),
            .collision_box = collisionBox,
        });
    }

    return result;
}

void SpriteSheet::createSpriteTexture(SDL_Renderer* renderer) {
    SOP_ASSERT(renderer != nullptr, "Asset manager requires a valid SDL renderer");
    SOP_ASSERT(m_SpriteTexture == nullptr, "Sprite texture should only be created once");
    SOP_ASSERT(m_SpriteSurface != nullptr, "Sprite surface should exist before creating texture");

    m_SpriteTexture = SDL_CreateTextureFromSurface(renderer, m_SpriteSurface);
    SOP_SDL_ASSERT(m_SpriteTexture != nullptr, "SDL_CreateTextureFromSurface");

    SDL_DestroySurface(m_SpriteSurface);
    m_SpriteSurface = nullptr;
}

void SpriteSheet::destroyOwnedResources() {
    if (m_SpriteSurface != nullptr) {
        SDL_DestroySurface(m_SpriteSurface);
        m_SpriteSurface = nullptr;
    }

    if (m_SpriteTexture != nullptr) {
        SDL_DestroyTexture(m_SpriteTexture);
        m_SpriteTexture = nullptr;
    }

    if (m_HitboxSheet != nullptr) {
        SDL_DestroySurface(m_HitboxSheet);
        m_HitboxSheet = nullptr;
    }
}
}  // namespace sop
