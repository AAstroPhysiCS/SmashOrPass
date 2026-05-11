#include "smashorpass/core/PlayerEffectEmitter.hpp"
#include "smashorpass/core/PlayerSpritePlacement.hpp"

namespace sop {

void PlayerEffectEmitter::Update(const PlayerCharacterState& player,
                                 CharacterAnimation animation,
                                 uint32_t frameIndex,
                                 const SpriteSheetFrame& frame,
                                 std::span<const FrameEffectMask> swordMasks,
                                 float renderScale,
                                 double dt,
                                 EventDispatcher& events) {
    m_DashEffectCooldown = std::max(0.0, m_DashEffectCooldown - dt);

    const bool frameChanged =
        animation != m_PreviousAnimation || frameIndex != m_PreviousFrameIndex;

    if (animation == CharacterAnimation::Attacks && frameChanged) {
        if (frameIndex < swordMasks.size()) {
            const FrameEffectMask& mask = swordMasks[frameIndex];

            if (!mask.Points.empty()) {
                constexpr int particlesPerAttackFrame = 5;

                for (int i = 0; i < particlesPerAttackFrame; ++i) {
                    const size_t pointIndex = static_cast<size_t>(std::rand()) % mask.Points.size();

                    const Vec2 localPoint = mask.Points[pointIndex];

                    const Vec2 worldPoint = detail::LocalFramePointToWorld(
                        player, frame, localPoint, player.FacingRight, renderScale);

                    events.Enqueue(PlayerParticleEffectEvent{
                        .Type = PlayerParticleEffectType::SwordFire,
                        .Position = worldPoint,
                        .Velocity = Vec2{0.0f, player.VerticalVelocity},
                        .FacingRight = player.FacingRight,
                        .Strength = 1.0f,
                    });
                }
            }
        }
    }

    if (animation == CharacterAnimation::Dash && m_DashEffectCooldown <= 0.0) {
        m_DashEffectCooldown = 0.015;

        const bool dashingRight = player.DashDirection > 0.0f;

        const Vec2 dashTrailPosition{
            dashingRight ? player.CollisionRect.x : player.CollisionRect.x + player.CollisionRect.w,
            player.CollisionRect.y + player.CollisionRect.h * 0.5f,
        };

        events.Enqueue(PlayerParticleEffectEvent{
            .Type = PlayerParticleEffectType::DashBlue,
            .Position = dashTrailPosition,
            .Velocity =
                Vec2{
                    dashingRight ? -player.DashDirection * 250.0f : -player.DashDirection * 250.0f,
                    0.0f,
                },
            .FacingRight = player.FacingRight,
            .Strength = 1.0f,
        });
    }

    m_PreviousAnimation = animation;
    m_PreviousFrameIndex = frameIndex;
}
}
