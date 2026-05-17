#pragma once

#include <random>
#include <vector>

#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

using namespace sop_util;

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
    ParticleSystem(ParticleSystem&&) = delete;
    ParticleSystem& operator=(ParticleSystem&&) = delete;

    Result<void> Initialize(const Renderer& renderer, size_t maxParticles = std::pow(2, 12));

    void EmitBurst(const ParticleBurstDesc& desc);

    void Clear();
    void Update(float dt);
    Result<void> Render(Renderer& renderer);

   private:
    Particle& GetFreeParticle();

    float RandomFloat(float min, float max);

   private:
    std::vector<Particle> m_Particles;
    size_t m_NextParticle = 0;
    SDL_Texture* m_ParticleTexture = nullptr;

    std::mt19937 m_Random;
};

}  // namespace sop
