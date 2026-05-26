#pragma once

#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

namespace sop {

using ChannelPlane = std::vector<std::vector<unsigned char>>;

struct GridData {
    SDL_FRect bounds;

    int matrixWidth;
    int matrixHeight;

    int cellSize;
    [[nodiscard]] int BucketMatrixWidth() const { return matrixWidth - 1; }
    [[nodiscard]] int BucketMatrixHeight() const { return matrixHeight - 1; }
    
    std::vector<SDL_Point> m_GridMatrix;
};

struct HitBox {
    GridData m_GridData;
    std::vector<std::vector<SDL_Point>> m_Buckets;
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

HitBox setupHitbox(const ChannelPlane& redChannel, int targetGridSize);
HurtBox setupHurtBox(const ChannelPlane& blueChannel, int targetGridSize);

} // namespace sop