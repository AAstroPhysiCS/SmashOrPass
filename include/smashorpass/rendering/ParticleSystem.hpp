#pragma once

#include <random>
#include <vector>

#include "smashorpass/core/Event.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

struct AppCtx;

struct Particle {
    Vec2 Position{};
    Vec2 Velocity{};
    Vec2 Acceleration{};

    Color StartColor{255, 255, 255, 255};
    Color EndColor{255, 255, 255, 0};

    float StartSize = 6.0f;
    float EndSize = 0.0f;

    float Lifetime = 1.0f;
    float RemainingLife = 0.0f;

    bool Active = false;
};

struct ParticleBurstDesc {
    Vec2 Position{};
    Vec2 InitialVelocity{};

    uint32_t Count = 16;

    float MinSpeed = 80.0f;
    float MaxSpeed = 220.0f;

    float MinLifetime = 0.25f;
    float MaxLifetime = 0.75f;

    float MinSize = 3.0f;
    float MaxSize = 8.0f;

    Color StartColor{255, 255, 255, 255};
    Color EndColor{255, 255, 255, 0};

    Vec2 Acceleration{0.0f, 300.0f};
};

class ParticleSystem {
   public:
    ParticleSystem() = default;
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    ParticleSystem(ParticleSystem&& other) noexcept
        : m_Particles(std::move(other.m_Particles)),
          m_NextParticle(std::exchange(other.m_NextParticle, 0)),
          m_ParticleTexture(std::exchange(other.m_ParticleTexture, nullptr)),
          m_Random(std::move(other.m_Random)) {}

    ParticleSystem& operator=(ParticleSystem&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (m_ParticleTexture != nullptr) {
            SDL_DestroyTexture(m_ParticleTexture);
        }

        m_Particles = std::move(other.m_Particles);
        m_NextParticle = std::exchange(other.m_NextParticle, 0);
        m_ParticleTexture = std::exchange(other.m_ParticleTexture, nullptr);
        m_Random = std::move(other.m_Random);

        return *this;
    }

    sop::Result<void, std::string> Initialize(const Renderer& renderer,
                                              size_t maxParticles = std::pow(2, 10));
    void EmitBurst(const ParticleBurstDesc& desc);

    void Clear();
    void Update(float dt);
    sop::Result<void> Render(AppCtx& ctx);

   private:
    Particle& GetFreeParticle();

    float RandomFloat(float min, float max);

   private:
    std::vector<Particle> m_Particles;
    size_t m_NextParticle = 0;
    SDL_Texture* m_ParticleTexture = nullptr;

    std::mt19937 m_Random;

    friend struct AppCtx;
};

namespace util {

inline void EmitSwordFireParticleEffect(ParticleSystem& particleSystem,
                                        const PlayerParticleEffectEvent& event) {
    ParticleBurstDesc desc{};
    desc.Position = event.Position;
    desc.InitialVelocity = Vec2{
        event.FacingRight ? -35.0f : 35.0f,
        -110.0f,
    };
    desc.Count = 1;
    desc.MinSpeed = 90.0f;
    desc.MaxSpeed = 230.0f;
    desc.MinLifetime = 1.2f;
    desc.MaxLifetime = 2.0f;
    desc.MinSize = 8.0f;
    desc.MaxSize = 20.0f;
    desc.StartColor = Color{40, 255, 0, 255};
    desc.EndColor = Color{255, 0, 0, 0};
    desc.Acceleration = Vec2{
        event.FacingRight ? -10.0f : 10.0f,
        -90.0f,
    };
    particleSystem.EmitBurst(desc);
}

inline void EmitDashParticleEffect(ParticleSystem& particleSystem,
                                   const PlayerParticleEffectEvent& event) {
    const float strength = std::max(event.Strength, 0.25f);

    // Main blue streak
    {
        ParticleBurstDesc desc{};
        desc.Position = event.Position;
        desc.InitialVelocity = event.Velocity;
        desc.Count = 8;
        desc.MinSpeed = 90.0f * strength;
        desc.MaxSpeed = 260.0f * strength;
        desc.MinLifetime = 0.22f;
        desc.MaxLifetime = 0.5f;
        desc.MinSize = 8.0f;
        desc.MaxSize = 22.0f;
        desc.StartColor = Color{80, 210, 255, 180};
        desc.EndColor = Color{20, 70, 255, 0};
        desc.Acceleration = Vec2{0.0f, 0.0f};

        particleSystem.EmitBurst(desc);
    }

    // Soft afterimage glow
    {
        ParticleBurstDesc desc{};
        desc.Position = event.Position;
        desc.InitialVelocity = Vec2{
            event.Velocity.x * 0.35f,
            event.Velocity.y * 0.35f,
        };
        desc.Count = 3;
        desc.MinSpeed = 20.0f * strength;
        desc.MaxSpeed = 80.0f * strength;
        desc.MinLifetime = 0.45f;
        desc.MaxLifetime = 0.8f;
        desc.MinSize = 18.0f;
        desc.MaxSize = 34.0f;
        desc.StartColor = Color{80, 160, 255, 80};
        desc.EndColor = Color{20, 40, 160, 0};
        desc.Acceleration = Vec2{0.0f, 0.0f};

        particleSystem.EmitBurst(desc);
    }
}
}  // namespace util
}  // namespace sop
