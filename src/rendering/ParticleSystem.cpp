#include "smashorpass/rendering/ParticleSystem.hpp"

#include <algorithm>
#include <cmath>

namespace sop {

ParticleSystem::ParticleSystem(size_t maxParticles)
    : m_Particles(maxParticles), m_Random(std::random_device{}()) {}

void ParticleSystem::EmitBurst(const ParticleBurstDesc& desc) {
    constexpr float twoPi = 6.28318530718f;

    for (uint32_t i = 0; i < desc.Count; ++i) {
        Particle& p = GetFreeParticle();

        const float angle = RandomFloat(0.0f, twoPi);
        const float speed = RandomFloat(desc.MinSpeed, desc.MaxSpeed);

        const Vec2 dir{std::cos(angle), std::sin(angle)};

        const float lifetime = RandomFloat(desc.MinLifetime, desc.MaxLifetime);
        const float size = RandomFloat(desc.MinSize, desc.MaxSize);

        p.Position = desc.Position;
        p.Velocity = Vec2{dir.x * speed, dir.y * speed};
        p.Acceleration = desc.Acceleration;

        p.StartColor = desc.StartColor;
        p.EndColor = desc.EndColor;

        p.StartSize = size;
        p.EndSize = 0.0f;

        p.Lifetime = lifetime;
        p.RemainingLife = lifetime;

        p.Active = true;
    }
}

void ParticleSystem::Update(float dt) {
    for (Particle& p : m_Particles) {
        if (!p.Active)
            continue;

        p.RemainingLife -= dt;

        if (p.RemainingLife <= 0.0f) {
            p.Active = false;
            continue;
        }

        p.Velocity.x += p.Acceleration.x * dt;
        p.Velocity.y += p.Acceleration.y * dt;

        p.Position.x += p.Velocity.x * dt;
        p.Position.y += p.Velocity.y * dt;
    }
}

void ParticleSystem::Render(Renderer& renderer) {
    for (const Particle& p : m_Particles) {
        if (!p.Active)
            continue;

        const float lifeRatio = p.RemainingLife / p.Lifetime;
        const float t = 1.0f - lifeRatio;

        const float size = Lerp(p.StartSize, p.EndSize, t);
        const Color color = LerpColor(p.StartColor, p.EndColor, t);

        const SDL_FRect rect{p.Position.x - size * 0.5f, p.Position.y - size * 0.5f, size, size};

        renderer.FillRect(rect, color);
    }
}

Particle& ParticleSystem::GetFreeParticle() {
    Particle& particle = m_Particles[m_NextParticle];
    m_NextParticle = (m_NextParticle + 1) % m_Particles.size();
    return particle;
}

float ParticleSystem::RandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_Random);
}

}  // namespace sop