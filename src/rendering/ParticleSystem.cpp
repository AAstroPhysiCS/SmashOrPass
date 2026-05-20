#include "smashorpass/rendering/ParticleSystem.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>

#include "SDL3/SDL_error.h"
#include "SDL3_image/SDL_image.h"
#include "smashorpass/core/AppCtx.hpp"

namespace sop {

Result<void> ParticleSystem::Initialize(const Renderer& renderer, size_t maxParticles) {
    if (maxParticles == 0) {
        return Err(std::string(
            "ParticleSystem::Initialize failed: maxParticles must be greater than zero"));
    }

    m_Particles.assign(maxParticles, Particle{});
    m_NextParticle = 0;
    m_Random = std::mt19937(std::random_device{}());

    auto path = std::filesystem::path(SOP_ASSET_ROOT_DIR) /
                "particles/soft_circle_particle_textures/soft_circle_particle_128.png";
    auto pathString = path.string();
    m_ParticleTexture = IMG_LoadTexture(renderer.NativeHandle(), pathString.c_str());
    if (m_ParticleTexture == nullptr) {
        return Err(
            std::format("Failed to load particle texture '{}': {}", pathString, SDL_GetError()));
    }

    if (!SDL_SetTextureBlendMode(m_ParticleTexture, SDL_BLENDMODE_BLEND)) {
        const std::string error = SdlError("SDL_SetTextureBlendMode");
        SDL_DestroyTexture(m_ParticleTexture);
        m_ParticleTexture = nullptr;
        return Err(error);
    }

    if (!SDL_SetTextureScaleMode(m_ParticleTexture, SDL_SCALEMODE_LINEAR)) {
        const std::string error = SdlError("SDL_SetTextureScaleMode");
        SDL_DestroyTexture(m_ParticleTexture);
        m_ParticleTexture = nullptr;
        return Err(error);
    }

    return Ok();
}

ParticleSystem::~ParticleSystem() {
    if (m_ParticleTexture != nullptr) {
        SDL_DestroyTexture(m_ParticleTexture);
        m_ParticleTexture = nullptr;
    }
}

void ParticleSystem::EmitBurst(const ParticleBurstDesc& desc) {
    if (m_Particles.empty()) {
        return;
    }

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
        p.Velocity = desc.InitialVelocity + p.Velocity;
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

void ParticleSystem::Clear() {
    for (Particle& particle : m_Particles) {
        particle.Active = false;
    }
    m_NextParticle = 0;
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

Result<void> ParticleSystem::Render(AppCtx& ctx) {
    Renderer& renderer = ctx.renderer;

    if (m_ParticleTexture == nullptr) {
        return Err(std::string("ParticleSystem::Render failed: particle texture is null"));
    }

    for (const Particle& p : m_Particles) {
        if (!p.Active)
            continue;

        const float lifeRatio = p.RemainingLife / p.Lifetime;
        const float t = 1.0f - lifeRatio;

        const float size = Lerp(p.StartSize, p.EndSize, t);
        const Color color = LerpColor(p.StartColor, p.EndColor, t);

        const SDL_FRect rect{p.Position.x - size * 0.5f, p.Position.y - size * 0.5f, size, size};

        // renderer.FillRect(rect, color);

        TRY_VOID(renderer.DrawTexture(m_ParticleTexture,
                                      TextureDrawParams{
                                          .dst = rect,
                                          .tint = color,
                                          .blendMode = SDL_BLENDMODE_BLEND,
                                      }));
    }
    return Ok();
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
