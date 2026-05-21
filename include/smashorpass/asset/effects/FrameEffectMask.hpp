#pragma once

#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

#include "smashorpass/core/Base.hpp"
#include "smashorpass/util.hpp"

namespace sop {

struct FrameEffectMask {
    std::vector<Vec2> Points;
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

template <typename T>
concept FrameEffectMaskType = std::is_enum_v<T>;

template <FrameEffectMaskType TType>
struct FrameEffectMaskDefinition {
    using PixelPredicate = bool (*)(const EffectMaskPixel&);  // using raw pointers to avoid unnecessary overhead of
    // std::function for this hot code path and i fucking hate clang format really
    TType Type{};
    uint32_t SampleStep = 4;
    PixelPredicate Predicate = nullptr;
};

template <FrameEffectMaskType TType>
class FrameEffectMaskSet final {
   public:
    using Type = TType;

    void Add(TType Type, std::vector<FrameEffectMask> masks) {
        m_Masks.insert_or_assign(Type, std::move(masks));
    }

    [[nodiscard]] std::span<const FrameEffectMask> Get(TType Type) const {
        const auto it = m_Masks.find(Type);
        if (it == m_Masks.end()) {
            return {};
        }
        return std::span<const FrameEffectMask>{
            it->second.data(),
            it->second.size(),
        };
    }

    [[nodiscard]] bool Contains(TType Type) const {
        return m_Masks.contains(Type);
    }

    void Clear() {
        m_Masks.clear();
    }
   private:
    std::unordered_map<TType, std::vector<FrameEffectMask>> m_Masks;
};
    
template <FrameEffectMaskType TType>
class FrameEffectMaskFactory final {
   public:
    using Type = TType;
    using Definition = FrameEffectMaskDefinition<TType>;
    using MaskSet = FrameEffectMaskSet<TType>;

    template <typename TFrame>
    [[nodiscard]] Result<MaskSet> Build(SDL_Surface* spriteSurface,
                                        std::span<const TFrame> frames,
                                        std::span<const Definition> definitions) const {
        MaskSet result{};

        for (const Definition& definition : definitions) {
            TRY_VOID(ValidateDefinition(definition));

            if (spriteSurface == nullptr) {
                return Err(
                    std::string{"FrameEffectMaskFactory::Build failed: spriteSurface surface was null."});
            }

            TRY(masks, BuildSingleMaskType(spriteSurface, frames, definition));
            result.Add(definition.Type, std::move(masks));
        }

        return Ok(std::move(result));
    }

   private:
    class SdlSurfaceDeleter final {
       public:
        void operator()(SDL_Surface* surface) const noexcept {
            if (surface != nullptr) {
                SDL_DestroySurface(surface);
            }
        }
    };

    [[nodiscard]] static Result<void> ValidateDefinition(const Definition& definition) {
        if (definition.SampleStep == 0) {
            return Err(std::string{
                "FrameEffectMaskFactory failed: SampleStep must be greater than zero."});
        }

        if (definition.Predicate == nullptr) {
            return Err(
                std::string{"FrameEffectMaskFactory failed: Pixel predicate must not be null."});
        }

        return Ok();
    }

    template <typename TFrame>
    [[nodiscard]] static Result<std::vector<FrameEffectMask>> BuildSingleMaskType(
        SDL_Surface* surface, std::span<const TFrame> frames, const Definition& definition) {
        std::vector<FrameEffectMask> masks;
        masks.resize(frames.size());

        std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> rgbaSurface{
            SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32)};

        if (rgbaSurface == nullptr) {
            return Err(SdlError("SDL_ConvertSurface"));
        }

        const bool mustLock = SDL_MUSTLOCK(rgbaSurface.get());

        if (mustLock && !SDL_LockSurface(rgbaSurface.get())) {
            return Err(SdlError("SDL_LockSurface"));
        }

        const auto unlockSurface = [&]() {
            if (mustLock) {
                SDL_UnlockSurface(rgbaSurface.get());
            }
        };

        const auto* pixels = static_cast<const uint8_t*>(rgbaSurface->pixels);

        for (std::size_t frameIndex = 0; frameIndex < frames.size(); ++frameIndex) {
            const TFrame& frame = frames[frameIndex];
            const SDL_FRect frameRect = frame.m_Location;

            FrameEffectMask& mask = masks[frameIndex];

            const auto left = static_cast<uint32_t>(frameRect.x);
            const auto top = static_cast<uint32_t>(frameRect.y);
            const auto right = static_cast<uint32_t>(frameRect.x + frameRect.w);
            const auto bottom = static_cast<uint32_t>(frameRect.y + frameRect.h);

            const auto clampedLeft =
                std::min<uint32_t>(left, static_cast<uint32_t>(rgbaSurface->w));

            const auto clampedTop =
                std::min<uint32_t>(top, static_cast<uint32_t>(rgbaSurface->h));

            const auto clampedRight =
                std::min<uint32_t>(right, static_cast<uint32_t>(rgbaSurface->w));

            const auto clampedBottom =
                std::min<uint32_t>(bottom, static_cast<uint32_t>(rgbaSurface->h));

            for (uint32_t y = clampedTop; y < clampedBottom; y += definition.SampleStep) {
                for (uint32_t x = clampedLeft; x < clampedRight; x += definition.SampleStep) {
                    const uint8_t* p = pixels + y * rgbaSurface->pitch + x * 4;

                    const EffectMaskPixel pixel{
                        .R = p[0],
                        .G = p[1],
                        .B = p[2],
                        .A = p[3],

                        .SheetX = x,
                        .SheetY = y,

                        .LocalX = x - clampedLeft,
                        .LocalY = y - clampedTop,

                        .FrameIndex = frameIndex,
                    };

                    if (!definition.Predicate(pixel)) {
                        continue;
                    }

                    mask.Points.push_back(Vec2{
                        static_cast<float>(pixel.LocalX),
                        static_cast<float>(pixel.LocalY),
                    });
                }
            }
        }

        unlockSurface();

        return Ok(std::move(masks));
    }
};

}  // namespace sop