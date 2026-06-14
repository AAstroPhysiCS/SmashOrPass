#pragma once

#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>
#include <nlohmann/json_fwd.hpp>

namespace sop {

using ChannelPlane = std::vector<std::vector<unsigned char>>;

struct GridData {
    SDL_FRect bounds{};

    int matrixWidth = 0;
    int matrixHeight = 0;

    int cellSize = 0;
    [[nodiscard]] int BucketMatrixWidth() const { return matrixWidth - 1; }
    [[nodiscard]] int BucketMatrixHeight() const { return matrixHeight - 1; }
    
    std::vector<SDL_Point> m_GridMatrix;
};

struct AttackData {
    int m_Id = 0;
    float m_Damage = 1.0f;
    SDL_FPoint m_Knockback{2.0f, -2.0f};
    int m_HitstunTicks = 6;
    int m_HitCooldownTicks = 10;
};

struct HitBox {
    GridData m_GridData;
    std::vector<std::vector<SDL_Point>> m_Buckets;
    AttackData m_AttackData;
};

struct SubHurtBox {
    GridData m_GridData;
    std::vector<std::vector<SDL_Point>> m_InnerBuckets;
    std::vector<std::vector<SDL_Point>> m_OuterBuckets;
};

struct HurtBox {
    // one SubHurtBox is stored per blue value range, damage is based on this
    // one for b=255 (head), one for b=127-254 (torso), one for b=1-127
    // we check the highest value one first for intersections etc.
    std::unordered_map<int, SubHurtBox> m_SubHurtBoxes;
};

HitBox setupHitbox(const ChannelPlane& redChannel, int targetGridSize, AttackData attackData);
HurtBox setupHurtBox(const ChannelPlane& blueChannel, int targetGridSize);
AttackData loadAttackData(const nlohmann::json& frameJson);

[[nodiscard]] inline bool IsEmpty(const HitBox& hitBox) {
    return hitBox.m_GridData.bounds.w <= 0.0f || hitBox.m_GridData.bounds.h <= 0.0f;
}
[[nodiscard]] inline bool IsEmpty(const HurtBox& hurtBox) {
    return hurtBox.m_SubHurtBoxes.empty();
}

} // namespace sop
