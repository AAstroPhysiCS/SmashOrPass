#include "smashorpass/asset/SpriteSheet.hpp"

#include <SDL3/SDL_iostream.h>
#include <SDL3_image/SDL_image.h>

#include <format>
#include <nlohmann/json.hpp>
#include <string>

namespace sop {

using namespace sop_util;

namespace {

Result<SDL_Surface*> LoadSurfaceFromBytes(std::span<const uint8_t> bytes, const char* name) {
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
        return Err(SdlError("SDL_CloseIO"));
    }
    if (surface == nullptr) {
        return Err(std::format("Failed to load PNG surface for {}: {}", name, loadError));
    }

    return Ok(surface);
}

}  // namespace

Result<SpriteSheet> SpriteSheet::parse(std::span<const uint8_t> spriteSheet,
                                       std::span<const uint8_t> hitboxSheet,
                                       std::span<const uint8_t> metadata) {
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

    TRY(spriteSurface, LoadSurfaceFromBytes(spriteSheet, "sprite sheet"));
    result.m_SpriteSurface = spriteSurface;
    TRY(hitboxSurface, LoadSurfaceFromBytes(hitboxSheet, "hitbox sheet"));
    result.m_HitboxSheet = hitboxSurface;

    try {
        const auto json = nlohmann::json::parse(metadata.begin(), metadata.end());
        const auto& framesJson = json.at("frames");
        if (!framesJson.is_array() || framesJson.empty()) {
            return Err(std::string("Sprite sheet metadata must contain a non-empty frames array"));
        }

        result.m_Frames.reserve(framesJson.size());

        for (const auto& frameJson : framesJson) {
            const auto& collisionBoxJson = frameJson.at("collision_box");
            const SDL_FRect collisionBox{
                .x = getFloat(collisionBoxJson, "x"),
                .y = getFloat(collisionBoxJson, "y"),
                .w = getFloat(collisionBoxJson, "width"),
                .h = getFloat(collisionBoxJson, "height"),
            };
            if (collisionBox.w < 0.0f || collisionBox.h < 0.0f) {
                return Err(std::string("Sprite collision box cannot be negative"));
            }

            SpriteSheetFrame frame{
                .x_left = getU32(frameJson, "x_left"),
                .x_right = getU32(frameJson, "x_right"),
                .y_top = getU32(frameJson, "y_top"),
                .y_bottom = getU32(frameJson, "y_bottom"),
                .anchor_x = getI32(frameJson, "anchor_x"),
                .anchor_y = getI32(frameJson, "anchor_y"),
                .collision_box = collisionBox,
            };
            if (frame.x_right <= frame.x_left || frame.y_bottom <= frame.y_top) {
                return Err(std::string("Sprite frame must have positive width and height"));
            }
            if (frame.x_right > static_cast<uint32_t>(result.m_SpriteSurface->w) ||
                frame.y_bottom > static_cast<uint32_t>(result.m_SpriteSurface->h)) {
                return Err(std::string("Sprite frame is outside the sprite sheet surface"));
            }
            result.m_Frames.push_back(frame);
        }
    } catch (const nlohmann::json::exception& e) {
        return Err(std::format("Failed to parse sprite sheet metadata: {}", e.what()));
    }

    return Ok(std::move(result));
}

Result<void> SpriteSheet::createSpriteTexture(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        return Err(std::string("SpriteSheet::createSpriteTexture failed: renderer is null"));
    }
    if (m_SpriteTexture != nullptr) {
        return Err(std::string("SpriteSheet::createSpriteTexture failed: texture already exists"));
    }
    if (m_SpriteSurface == nullptr) {
        return Err(std::string("SpriteSheet::createSpriteTexture failed: sprite surface is null"));
    }

    m_SpriteTexture = SDL_CreateTextureFromSurface(renderer, m_SpriteSurface);
    if (m_SpriteTexture == nullptr) {
        return Err(SdlError("SDL_CreateTextureFromSurface"));
    }

    SDL_DestroySurface(m_SpriteSurface);
    m_SpriteSurface = nullptr;
    return Ok();
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
