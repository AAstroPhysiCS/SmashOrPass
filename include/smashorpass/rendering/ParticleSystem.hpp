#pragma once

#include <random>
#include <vector>

#include "smashorpass/core/Base.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

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
    explicit ParticleSystem(size_t maxParticles = 512);
    ~ParticleSystem() = default;

    void EmitBurst(const ParticleBurstDesc& desc);

    void Update(float dt);
    void Render(Renderer& renderer);
   private:
    Particle& GetFreeParticle();

    float RandomFloat(float min, float max);
   private:
    std::vector<Particle> m_Particles;
    size_t m_NextParticle = 0;

    std::mt19937 m_Random;
};

}  // namespace sop