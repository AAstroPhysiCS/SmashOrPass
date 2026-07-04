#include "smashorpass/asset/assets/CharacterAsset.hpp"

#include <SDL3/SDL_pixels.h>
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"

namespace sop {

static constexpr std::array kCharacterAnimations{
    CharacterAnimation::Idle,
    CharacterAnimation::Walk,
    CharacterAnimation::Ascending,
    CharacterAnimation::Falling,
    CharacterAnimation::Attacks,
    CharacterAnimation::Dash,
};

std::string_view CharacterAnimationName(CharacterAnimation animation) {
    switch (animation) {
        case CharacterAnimation::Idle:
            return "Idle";
        case CharacterAnimation::Walk:
            return "Walk";
        case CharacterAnimation::Ascending:
            return "Ascending";
        case CharacterAnimation::Falling:
            return "Falling";
        case CharacterAnimation::Attacks:
            return "Attacks";
        case CharacterAnimation::Dash:
            return "Dash";
    }

    return "Unknown";
}

static Result<void> ConfigureTexture(SDL_Texture* texture, std::string_view name) {
    if (texture == nullptr) {
        return Err(std::format("ConfigureTexture failed: {} texture is null", name));
    }

    TRY_VOID(SdlResult(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND),
                       std::format("SDL_SetTextureBlendMode {}", name)));
    TRY_VOID(SdlResult(SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST),
                       std::format("SDL_SetTextureScaleMode {}", name)));
    return Ok();
}

static TexturePtr CreateTextureFromSurface(AppCtx& ctx,
                                           SDL_Surface* surface,
                                           std::string_view name) {
    if (surface == nullptr) {
        spdlog::warn("Cannot create texture for '{}': surface is null", name);
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    TexturePtr texture{
        SDL_CreateTextureFromSurface(ctx.renderer.NativeHandle(), surface),
        SDL_DestroyTexture,
    };
    if (!texture) {
        spdlog::warn("Failed to create texture for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    if (auto result = ConfigureTexture(texture.get(), name); !result) {
        spdlog::warn("Failed to configure texture for '{}': {}", name, result.error());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    return texture;
}

static TexturePtr CreateFilledTexture(AppCtx& ctx,
                                      int width,
                                      int height,
                                      Uint8 red,
                                      Uint8 green,
                                      Uint8 blue,
                                      Uint8 alpha,
                                      std::string_view name) {
    SurfacePtr surface{
        SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface,
    };
    if (!surface) {
        spdlog::warn("Failed to create fallback surface for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    if (!SDL_FillSurfaceRect(
            surface.get(), nullptr, SDL_MapSurfaceRGBA(surface.get(), red, green, blue, alpha))) {
        spdlog::warn("Failed to fill fallback surface for '{}': {}", name, SDL_GetError());
        return TexturePtr{nullptr, SDL_DestroyTexture};
    }

    return CreateTextureFromSurface(ctx, surface.get(), name);
}

static Result<SurfacePtr> LoadSurface(const std::filesystem::path& path, std::string_view name) {
    const std::string pathString = path.string();
    SurfacePtr surface{IMG_Load(pathString.c_str()), SDL_DestroySurface};
    if (!surface) {
        return Err(std::format("Failed to load {} '{}': {}", name, pathString, SDL_GetError()));
    }

    return Ok(std::move(surface));
}

static Result<std::vector<CharacterSpriteSheetFrame>> LoadCharacterFrames(
    const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Err(std::format("Failed to open character metadata '{}'", path.string()));
    }

    const auto getFloat = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<float>();
    };
    const auto getInt = [](const nlohmann::json& j, const char* key) {
        return j.at(key).get<int>();
    };

    try {
        const nlohmann::json json = nlohmann::json::parse(file);
        const auto& framesJson = json.at("frames");
        if (!framesJson.is_array() || framesJson.empty()) {
            return Err(std::string("Character metadata frames field must be a non-empty array"));
        }

        std::vector<CharacterSpriteSheetFrame> frames;
        frames.reserve(framesJson.size());

        for (const auto& frameJson : framesJson) {
            const float xLeft = getFloat(frameJson, "x_left");
            const float xRight = getFloat(frameJson, "x_right");
            const float yTop = getFloat(frameJson, "y_top");
            const float yBottom = getFloat(frameJson, "y_bottom");
            const auto& collisionBoxJson = frameJson.at("collision_box");

            frames.push_back(CharacterSpriteSheetFrame{
                .m_Location =
                    SDL_FRect{.x = xLeft, .y = yTop, .w = xRight - xLeft, .h = yBottom - yTop},
                .m_Anchor = SDL_Point{.x = getInt(frameJson, "anchor_x"),
                                      .y = getInt(frameJson, "anchor_y")},
                .m_CollisionBox =
                    SDL_FRect{
                        .x = getFloat(collisionBoxJson, "x"),
                        .y = getFloat(collisionBoxJson, "y"),
                        .w = getFloat(collisionBoxJson, "width"),
                        .h = getFloat(collisionBoxJson, "height"),
                    },
                .m_HitBox =
                    HitBox{
                        .m_GridData = GridData{},
                        .m_Buckets = {},
                        .m_AttackData = loadAttackData(frameJson),
                    },
                .m_HurtBox = HurtBox{},
            });
        }

        return Ok(std::move(frames));
    } catch (const nlohmann::json::exception& e) {
        return Err(
            std::format("Failed to parse character metadata '{}': {}", path.string(), e.what()));
    }
}

static Result<std::vector<std::string>> ListDirectoryIds(const std::filesystem::path& dir) {
    std::error_code ec;
    const bool exists = std::filesystem::exists(dir, ec);
    if (ec) {
        return Err(
            std::format("Failed to inspect asset directory '{}': {}", dir.string(), ec.message()));
    }
    if (!exists) {
        return Ok(std::vector<std::string>{});
    }

    std::vector<std::string> ids;
    for (std::filesystem::directory_iterator it{dir, ec}, end; !ec && it != end; it.increment(ec)) {
        if (!it->is_directory(ec) || ec) {
            continue;
        }

        const std::string assetId = it->path().filename().string();
        if (!assetId.empty() && std::ranges::find(ids, assetId) == ids.end()) {
            ids.push_back(assetId);
        }
    }

    if (ec) {
        return Err(
            std::format("Failed to scan asset directory '{}': {}", dir.string(), ec.message()));
    }

    std::ranges::sort(ids);
    return Ok(std::move(ids));
}

static CharacterSpriteSheet CreateDefaultCharacterSpriteSheet(AppCtx& ctx,
                                                              CharacterAnimation animation) {
    CharacterSpriteSheet sheet{};
    sheet.m_Texture = CreateFilledTexture(
        ctx, 292, 315, 0, 0, 0, 255, std::format("fallback {}", CharacterAnimationName(animation)));
    sheet.m_Frames.push_back(CharacterSpriteSheetFrame{
        .m_Location = SDL_FRect{.x = 0.0f, .y = 0.0f, .w = 292.0f, .h = 315.0f},
        .m_Anchor = SDL_Point{.x = 93, .y = -77},
        .m_CollisionBox = SDL_FRect{.x = 0.0f, .y = 0.0f, .w = 262.0f, .h = 285.0f},
        .m_HitBox = HitBox{},
        .m_HurtBox = HurtBox{},
    });
    return sheet;
}

CharacterAssetData CharacterAssetData::Default(AppCtx& ctx, const CharacterAssetLoadJob& loadJob) {
    CharacterAssetData asset{};
    asset.m_Id = loadJob.m_Id;
    for (const CharacterAnimation animation : kCharacterAnimations) {
        asset.m_SpriteSheets.try_emplace(animation,
                                         CreateDefaultCharacterSpriteSheet(ctx, animation));
    }
    return asset;
}

Result<std::vector<CharacterAssetLoadJob>> CharacterAssetDiscoverer::ListAvailableAssets(
    AppCtx& ctx) {
    TRY(ids, ListDirectoryIds(ctx.assetRootDir / "sprites" / "characters"));

    std::vector<CharacterAssetLoadJob> jobs;
    jobs.reserve(ids.size());
    for (std::string& id : ids) {
        jobs.push_back(CharacterAssetLoadJob{.m_Id = std::move(id)});
    }

    return Ok(std::move(jobs));
}

namespace {

// RAII for LoadCombatData
class SurfaceLock final {
   public:
    explicit SurfaceLock(SDL_Surface* surface) : m_Surface(surface) {}

    SurfaceLock(const SurfaceLock&) = delete;
    SurfaceLock& operator=(const SurfaceLock&) = delete;

    [[nodiscard]] Result<void> LockIfNeeded() {
        if (m_Surface == nullptr) {
            return Err(std::string("SurfaceLock failed: surface is null"));
        }

        if (!SDL_MUSTLOCK(m_Surface)) {
            return Ok();
        }

        if (!SDL_LockSurface(m_Surface)) {
            return Err(SdlError("SDL_LockSurface"));
        }

        m_Locked = true;
        return Ok();
    }

    ~SurfaceLock() {
        if (m_Locked) {
            SDL_UnlockSurface(m_Surface);
        }
    }

   private:
    SDL_Surface* m_Surface = nullptr;
    bool m_Locked = false;
};

Result<SurfacePtr> ConvertToRgba32(SDL_Surface* surface) {
    if (surface == nullptr) {
        return Err(std::string("ConvertToRgba32 failed: surface is null"));
    }

    SurfacePtr rgbaSurface{SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface};

    if (!rgbaSurface) {
        return Err(SdlError("SDL_ConvertSurface"));
    }

    return Ok(std::move(rgbaSurface));
}

ChannelPlane getFrameChannelFromSurface(const unsigned char* pixels,
                                        const int pitch,
                                        const SDL_FRect& framePosition,
                                        const int channelOffset,
                                        const bool isHitbox) {
    const int left = static_cast<int>(framePosition.x);
    const int top = static_cast<int>(framePosition.y);
    const int width = static_cast<int>(framePosition.w);
    const int height = static_cast<int>(framePosition.h);

    ChannelPlane channel(width, std::vector<unsigned char>(height));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int pixelsX = left + x;
            const int pixelsY = top + y;
            const uint8_t* p = pixels + pixelsY * pitch + pixelsX * 4;

            channel[x][y] =
                static_cast<unsigned char>((static_cast<int>(p[channelOffset]) * p[3]) / 255);

            // hitboxes are absolute, hurtboxes are grouped by their blue channel value
            // each group gets the same amount of damage (3=head -> max damage etc.)
            if (channel[x][y] != 0) {
                if (isHitbox) {
                    channel[x][y] = 1;
                } else {
                    if (channel[x][y] == 255) {
                        channel[x][y] = 3;
                    } else if (channel[x][y] > 127) {
                        channel[x][y] = 2;
                    } else {
                        channel[x][y] = 1;
                    }
                }
            }
        }
    }

    return channel;
}

}  // namespace

Result<void> LoadCombatData(SDL_Surface* combatSurface,
                            std::vector<CharacterSpriteSheetFrame>& frames,
                            int gridSize) {
    TRY(rgbaSurface, ConvertToRgba32(combatSurface));

    SurfaceLock surfaceLock{rgbaSurface.get()};
    TRY_VOID(surfaceLock.LockIfNeeded());

    const auto* pixels = static_cast<const unsigned char*>(rgbaSurface.get()->pixels);
    const int pitch = rgbaSurface.get()->pitch;

    for (auto& frame : frames) {
        const ChannelPlane red =
            getFrameChannelFromSurface(pixels, pitch, frame.m_Location, 0, true);
        frame.m_HitBox = setupHitbox(red, gridSize, frame.m_HitBox.m_AttackData);

        const ChannelPlane blue =
            getFrameChannelFromSurface(pixels, pitch, frame.m_Location, 2, false);
        frame.m_HurtBox = setupHurtBox(blue, gridSize);
    }

    return Ok();
}

static Result<RawCharacterSpriteSheet> LoadCharacterSpriteSheet(
    const std::filesystem::path& characterDir, CharacterAnimation animation) {
    const std::string animationName{CharacterAnimationName(animation)};
    TRY(surface, LoadSurface(characterDir / (animationName + ".png"), animationName));
    TRY(combatSurface, LoadSurface(characterDir / (animationName + "_boxes.png"), animationName));
    TRY(frames, LoadCharacterFrames(characterDir / (animationName + ".json")));

    return Ok(RawCharacterSpriteSheet{
        .m_Animation = animation,
        .m_Surface = std::move(surface),
        .m_CombatSurface = std::move(combatSurface),
        .m_Frames = std::move(frames),
    });
}

RawCharacterAssetData CharacterAssetLoadJob::ToRawAssetData(AppCtx& ctx) {
    RawCharacterAssetData raw{};
    raw.m_Id = m_Id;
    if (m_Id.empty()) {
        raw.m_Error = "No character asset id provided. Using fallback character asset.";
        return raw;
    }

    const std::filesystem::path characterDir = ctx.assetRootDir / "sprites" / "characters" / m_Id;

    for (const CharacterAnimation animation : kCharacterAnimations) {
        auto sheet = LoadCharacterSpriteSheet(characterDir, animation);
        if (!sheet) {
            spdlog::warn("Failed to load character asset '{}', animation '{}': {}",
                         m_Id,
                         CharacterAnimationName(animation),
                         sheet.error());
            continue;
        }

        raw.m_SpriteSheets.try_emplace(animation, std::move(*sheet));
    }

    if (raw.m_SpriteSheets.empty()) {
        raw.m_Error = "No character animation sheets could be loaded.";
    }

    return raw;
}

CharacterAssetData RawCharacterAssetData::ToAssetData(AppCtx& ctx) {
    CharacterAssetData asset =
        CharacterAssetData::Default(ctx, CharacterAssetLoadJob{.m_Id = m_Id});
    if (!m_Error.empty()) {
        spdlog::warn("Failed to load character asset '{}': {}", m_Id, m_Error);
        return asset;
    }

    for (auto& [animation, rawSheet] : m_SpriteSheets) {
        TexturePtr texture = CreateTextureFromSurface(
            ctx, rawSheet.m_Surface.get(), CharacterAnimationName(animation));
        if (!texture) {
            spdlog::warn("Failed to create texture for character asset '{}', animation '{}'",
                         m_Id,
                         CharacterAnimationName(animation));
            continue;
        }

        auto combatData = LoadCombatData(rawSheet.m_CombatSurface.get(), rawSheet.m_Frames, 50);
        if (!combatData) {
            spdlog::warn("Failed to build combat data for character asset '{}', animation '{}': {}",
                         m_Id,
                         CharacterAnimationName(animation),
                         combatData.error());
            continue;
        }

        CharacterFrameEffectMasks::Factory effectMaskFactory{};

        auto effectMasks =
            effectMaskFactory.Build(rawSheet.m_Surface.get(),
                                    std::span<const CharacterSpriteSheetFrame>{
                                        rawSheet.m_Frames.data(), rawSheet.m_Frames.size()},
                                    CharacterFrameEffectMasks::Definitions());
        if (!effectMasks) {
            spdlog::warn(
                "Failed to build frame effect masks for character asset '{}', animation '{}': {}",
                m_Id,
                CharacterAnimationName(animation),
                effectMasks.error());
            continue;
        }
        CharacterSpriteSheet sheet{};
        sheet.m_Texture = std::move(texture);
        sheet.m_Frames = std::move(rawSheet.m_Frames);
        sheet.m_EffectMasks = std::move(effectMasks.value());
        asset.m_SpriteSheets.insert_or_assign(animation, std::move(sheet));
    }

    return asset;
}

}  // namespace sop
