#pragma once

#include <cstddef>
#include <cstdint>

#include "smashorpass/asset/AssetManager.hpp"
#include "smashorpass/core/Base.hpp"

namespace sop {

class SpriteAnimationPlayer final {
   public:
    explicit SpriteAnimationPlayer(CharacterAnimation animation = CharacterAnimation::Idle)
        : m_Animation(animation) {}

    void SetAnimation(CharacterAnimation animation) {
        if (m_Animation == animation) {
            return;
        }

        m_Animation = animation;
        m_FrameIndex = 0;
        m_TickCount = 0;
    }

    void Advance(std::size_t frameCount) {
        SOP_ASSERT(frameCount > 0, "Sprite animation requires at least one frame");

        m_FrameIndex = (m_FrameIndex + 1U) % frameCount;
        ++m_TickCount;
    }

    [[nodiscard]] CharacterAnimation GetAnimation() const {
        return m_Animation;
    }
    [[nodiscard]] std::size_t GetFrameIndex() const {
        return m_FrameIndex;
    }
    [[nodiscard]] uint64_t GetTickCount() const {
        return m_TickCount;
    }

   private:
    CharacterAnimation m_Animation = CharacterAnimation::Idle;
    std::size_t m_FrameIndex = 0;
    uint64_t m_TickCount = 0;
};
}  // namespace sop
