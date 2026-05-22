#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "smashorpass/asset/effects/FrameEffectMask.hpp"

namespace sop {

enum class CharacterFrameEffectMaskType : std::uint8_t {
    SwordFire,
    HitSpark,
    DashTrail,
    FootDust,
    LandingDust,
};

class CharacterFrameEffectMasks final {
   public:
    using Type = CharacterFrameEffectMaskType;
    using Definition = FrameEffectMaskDefinition<Type>;
    using Set = FrameEffectMaskSet<Type>;
    using Factory = FrameEffectMaskFactory<Type>;

    [[nodiscard]] static std::span<const Definition> Definitions() {
        static constexpr std::array<Definition, 1> definitions{
            Definition{
                .Type = Type::SwordFire,
                .SampleStep = 4,
                .Predicate =
                    [](const EffectMaskPixel& pixel) {
                        const float r = static_cast<float>(pixel.R);
                        const float g = static_cast<float>(pixel.G);
                        const float b = static_cast<float>(pixel.B);

                        return pixel.A > 64 && g > 110.0f && g > r * 1.25f && g > b * 1.25f;
                    },
            },
        };

        return std::span<const Definition>{
            definitions.data(),
            definitions.size(),
        };
    }
};

}  // namespace sop