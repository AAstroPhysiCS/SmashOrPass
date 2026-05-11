#pragma once

#include "PlayerController.hpp"

namespace sop {

class PlayerEffectEmitter final {
   public:
    void Update(const PlayerCharacterState& player,
                CharacterAnimation animation,
                uint32_t frameIndex,
                const SpriteSheetFrame& frame,
                std::span<const FrameEffectMask> swordMasks,
                float renderScale,
                double dt,
                EventDispatcher& events);

   private:
    CharacterAnimation m_PreviousAnimation = CharacterAnimation::Idle;
    uint32_t m_PreviousFrameIndex = std::numeric_limits<uint32_t>::max();
    double m_DashEffectCooldown = 0.0;
};
}