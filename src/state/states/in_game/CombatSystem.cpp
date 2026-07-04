#include "smashorpass/state/states/in_game/CombatSystem.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

#include "smashorpass/state/states/in_game/Defaults.hpp"

namespace sop {

namespace {

std::uint64_t packPoint(int x, int y) {
    // combine x and y for the hashmap
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32) |
           static_cast<std::uint32_t>(y);
}

struct IntersectionInfo {
    bool overlaps = false;
    SDL_FRect rect{};
};

struct DefinedHitbox {
    std::unordered_map<std::uint64_t, bool> attackPixels;
    std::unordered_map<int, SDL_FRect> bucketRectsByHurtValue;
};

struct OrderedBucket {
    int index = 0;
    float distanceSquared = 0.0f;
};

struct BucketRange {
    int startX = 0;
    int endX = 0;
    int startY = 0;
    int endY = 0;
};

IntersectionInfo intersects(const SDL_FRect& hitbox, const SDL_FRect& hurtbox) {
    const float left = std::max(hitbox.x, hurtbox.x);
    const float top = std::max(hitbox.y, hurtbox.y);
    const float right = std::min(hitbox.x + hitbox.w, hurtbox.x + hurtbox.w);
    const float bottom = std::min(hitbox.y + hitbox.h, hurtbox.y + hurtbox.h);

    if (left >= right || top >= bottom) {
        return IntersectionInfo{};
    }

    IntersectionInfo info;
    info.overlaps = true;
    info.rect = SDL_FRect{
        left,
        top,
        right - left,
        bottom - top,
    };

    return info;
}

SDL_FRect transformRectToWorldspace(const SDL_FRect& localRect,
                                    const SDL_FRect& spriteRect,
                                    const bool isFacingRight) {
    const float scaledX = localRect.x * kPlayerScale;
    const float scaledY = localRect.y * kPlayerScale;
    const float scaledW = localRect.w * kPlayerScale;
    const float scaledH = localRect.h * kPlayerScale;

    return SDL_FRect{isFacingRight ? (spriteRect.x + spriteRect.w - scaledX - scaledW)
                                   : (spriteRect.x + scaledX),
                     spriteRect.y + scaledY,
                     scaledW,
                     scaledH};
}

SDL_Point transformPointToWorldSpace(const SDL_Point& localPoint,
                                     const SDL_FRect& localBounds,
                                     const SDL_FRect& worldBounds,
                                     const bool isFacingRight) {
    const float scaledBoundsX = localBounds.x * kPlayerScale;
    const float scaledBoundsY = localBounds.y * kPlayerScale;
    const float scaledPointX = static_cast<float>(localPoint.x) * kPlayerScale;
    const float scaledPointY = static_cast<float>(localPoint.y) * kPlayerScale;

    return SDL_Point{
        static_cast<int>(std::lround(isFacingRight ? worldBounds.x + worldBounds.w - scaledBoundsX -
                                                         scaledPointX - kPlayerScale
                                                   : worldBounds.x + scaledBoundsX + scaledPointX)),
        static_cast<int>(std::lround(worldBounds.y + scaledBoundsY + scaledPointY)),
    };
}

SDL_FRect getHitboxRect(const HitBox& attackHitBox,
                        const SDL_FRect& attackWorldBounds,
                        const bool isFacingRight,
                        int startBucketX,
                        int startBucketY,
                        int endBucketX,
                        int endBucketY) {
    const float bucketLocalLeft =
        static_cast<float>(startBucketX * attackHitBox.m_GridData.cellSize) * kPlayerScale;
    const float bucketLocalTop =
        static_cast<float>(startBucketY * attackHitBox.m_GridData.cellSize) * kPlayerScale;
    const float bucketLocalRight = std::min(
        static_cast<float>((endBucketX + 1) * attackHitBox.m_GridData.cellSize) * kPlayerScale,
        attackHitBox.m_GridData.bounds.w * kPlayerScale);
    const float bucketLocalBottom = std::min(
        static_cast<float>((endBucketY + 1) * attackHitBox.m_GridData.cellSize) * kPlayerScale,
        attackHitBox.m_GridData.bounds.h * kPlayerScale);

    return SDL_FRect{
        isFacingRight ? (attackWorldBounds.x + attackWorldBounds.w - bucketLocalRight)
                      : (attackWorldBounds.x + bucketLocalLeft),
        attackWorldBounds.y + bucketLocalTop,
        bucketLocalRight - bucketLocalLeft,
        bucketLocalBottom - bucketLocalTop,
    };
}

BucketRange getOverlappingBucketRange(const SDL_FRect& intersection,
                                      const SDL_FRect& worldBounds,
                                      const GridData& grid,
                                      const bool isFacingRight) {
    // Grid cell size, scaled to the players. Used for bucket overlap computations below.
    const float scaledCellSize = static_cast<float>(grid.cellSize) * kPlayerScale;

    // Intersection rectangle coordinates in the hitbox/hurtbox local grid space.
    const float localLeft = intersection.x - worldBounds.x;
    const float localTop = intersection.y - worldBounds.y;
    const float localRight = localLeft + intersection.w;
    const float localBottom = localTop + intersection.h;

    // Select all buckets that are inside the intersection rectangle.
    BucketRange range{};
    if (isFacingRight) {
        const float mirroredLeft = worldBounds.w - localRight;
        const float mirroredRight = worldBounds.w - localLeft;
        range.startX = std::max(0, static_cast<int>(std::floor(mirroredLeft / scaledCellSize)));
        range.endX =
            std::min(grid.BucketMatrixWidth() - 1,
                     static_cast<int>(std::floor((mirroredRight - 1.0f) / scaledCellSize)));
    } else {
        range.startX = std::max(0, static_cast<int>(std::floor(localLeft / scaledCellSize)));
        range.endX = std::min(grid.BucketMatrixWidth() - 1,
                              static_cast<int>(std::floor((localRight - 1.0f) / scaledCellSize)));
    }

    range.startY = std::max(0, static_cast<int>(std::floor(localTop / scaledCellSize)));
    range.endY = std::min(grid.BucketMatrixHeight() - 1,
                          static_cast<int>(std::floor((localBottom - 1.0f) / scaledCellSize)));

    return range;
}

std::vector<int> getOrderedBuckets(const SubHurtBox& defenderSubHurtBox,
                                   const SDL_FRect& defenderWorldBounds,
                                   const IntersectionInfo& intersection,
                                   const std::vector<uint8_t>& neededBuckets,
                                   const bool defenderFacingRight,
                                   int startBucketX,
                                   int startBucketY,
                                   int endBucketX,
                                   int endBucketY) {
    // builds a list of hurtbox bucket indices, based on how close each bucket is to the center of
    // the hitbox
    const float overlapCenterX = intersection.rect.x + intersection.rect.w * 0.5f;
    const float overlapCenterY = intersection.rect.y + intersection.rect.h * 0.5f;

    std::vector<OrderedBucket> orderedBuckets;
    orderedBuckets.reserve((endBucketX - startBucketX + 1) * (endBucketY - startBucketY + 1));

    const float cellSize =
        static_cast<float>(defenderSubHurtBox.m_GridData.cellSize) * kPlayerScale;

    for (int bucketY = startBucketY; bucketY <= endBucketY; bucketY++) {
        for (int bucketX = startBucketX; bucketX <= endBucketX; bucketX++) {
            const int bucketIndex =
                bucketY * defenderSubHurtBox.m_GridData.BucketMatrixWidth() + bucketX;
            if (!neededBuckets[bucketIndex]) {
                continue;
            }

            const float localBucketCenterX =
                static_cast<float>(bucketX) * cellSize + cellSize * 0.5f;

            const float bucketCenterX =
                defenderFacingRight
                    ? defenderWorldBounds.x + defenderWorldBounds.w - localBucketCenterX
                    : defenderWorldBounds.x + localBucketCenterX;

            const float bucketCenterY =
                defenderWorldBounds.y + static_cast<float>(bucketY) * cellSize + cellSize * 0.5f;
            const float dx = bucketCenterX - overlapCenterX;
            const float dy = bucketCenterY - overlapCenterY;

            orderedBuckets.push_back(OrderedBucket{bucketIndex, dx * dx + dy * dy});
        }
    }

    std::sort(orderedBuckets.begin(),
              orderedBuckets.end(),
              [](const OrderedBucket& left, const OrderedBucket& right) {
                  return left.distanceSquared < right.distanceSquared;
              });

    std::vector<int> orderedBucketIndices;
    orderedBucketIndices.reserve(orderedBuckets.size());
    for (const OrderedBucket& orderedBucket : orderedBuckets) {
        orderedBucketIndices.push_back(orderedBucket.index);
    }

    return orderedBucketIndices;
}

DefinedHitbox defineHitbox(const HitBox& attackerHitBox,
                           const SDL_FRect& attackerSpriteRect,
                           const bool attackerFacingRight,
                           const HurtBox& defenderHurtBox,
                           const SDL_FRect& defenderSpriteRect,
                           const bool defenderFacingRight) {
    const SDL_FRect attackWorldBounds = transformRectToWorldspace(
        attackerHitBox.m_GridData.bounds, attackerSpriteRect, attackerFacingRight);
    const int totalBuckets = attackerHitBox.m_GridData.BucketMatrixWidth() *
                             attackerHitBox.m_GridData.BucketMatrixHeight();
    std::vector<uint8_t> neededBuckets(totalBuckets, 0);
    std::unordered_map<int, SDL_FRect> bucketRectsByHurtValue;

    // check for each subHurtBox, where it overlaps with the Hitbox
    // then only select the Overlapped Grid Cells (Buckets)
    for (const auto& [value, subHurtBox] : defenderHurtBox.m_SubHurtBoxes) {
        const SDL_FRect subWorldBounds = transformRectToWorldspace(
            subHurtBox.m_GridData.bounds, defenderSpriteRect, defenderFacingRight);
        const IntersectionInfo intersection = intersects(attackWorldBounds, subWorldBounds);
        if (!intersection.overlaps) {
            continue;
        }

        const BucketRange bucketRange = getOverlappingBucketRange(
            intersection.rect, attackWorldBounds, attackerHitBox.m_GridData, attackerFacingRight);

        bucketRectsByHurtValue[value] = getHitboxRect(attackerHitBox,
                                                      attackWorldBounds,
                                                      attackerFacingRight,
                                                      bucketRange.startX,
                                                      bucketRange.startY,
                                                      bucketRange.endX,
                                                      bucketRange.endY);

        for (int bucketY = bucketRange.startY; bucketY <= bucketRange.endY; bucketY++) {
            for (int bucketX = bucketRange.startX; bucketX <= bucketRange.endX; bucketX++) {
                neededBuckets[bucketY * attackerHitBox.m_GridData.BucketMatrixWidth() + bucketX] =
                    1;
            }
        }
    }

    // create a hashmap of all the pixels, that intersect with a hurtbox
    DefinedHitbox definedHitbox;
    definedHitbox.bucketRectsByHurtValue = std::move(bucketRectsByHurtValue);
    definedHitbox.attackPixels.reserve(4096);

    for (int bucketIndex = 0; bucketIndex < totalBuckets; bucketIndex++) {
        if (!neededBuckets[bucketIndex]) {
            continue;
        }

        for (const SDL_Point& point : attackerHitBox.m_Buckets[bucketIndex]) {
            const SDL_Point worldPoint = transformPointToWorldSpace(
                point, attackerHitBox.m_GridData.bounds, attackerSpriteRect, attackerFacingRight);
            definedHitbox.attackPixels[packPoint(worldPoint.x, worldPoint.y)] = true;
        }
    }

    return definedHitbox;
}

bool isInHitbox(const std::unordered_map<std::uint64_t, bool>& attackPixels,
                const std::vector<SDL_Point>& bucket,
                const SDL_FRect& defenderBounds,
                const SDL_FRect& defenderSpriteRect,
                const bool defenderFacingRight) {
    for (const SDL_Point& point : bucket) {
        const SDL_Point worldPoint = transformPointToWorldSpace(
            point, defenderBounds, defenderSpriteRect, defenderFacingRight);
        if (attackPixels.find(packPoint(worldPoint.x, worldPoint.y)) != attackPixels.end()) {
            return true;
        }
    }
    return false;
}

bool checkIfHurtBoxWasHit(const std::unordered_map<std::uint64_t, bool>& attackPixels,
                          const SDL_FRect& hitboxRect,
                          const SubHurtBox& defenderSubHurtBox,
                          const SDL_FRect& defenderSpriteRect,
                          const bool defenderFacingRight,
                          PlayerCombatDebugData* defenderDebugData) {
    const SDL_FRect defenderWorldBounds = transformRectToWorldspace(
        defenderSubHurtBox.m_GridData.bounds, defenderSpriteRect, defenderFacingRight);
    const int totalBuckets = defenderSubHurtBox.m_GridData.BucketMatrixWidth() *
                             defenderSubHurtBox.m_GridData.BucketMatrixHeight();
    std::vector<uint8_t> neededBuckets(totalBuckets, 0);

    const IntersectionInfo intersection = intersects(defenderWorldBounds, hitboxRect);
    if (!intersection.overlaps) {
        return false;
    }

    const BucketRange bucketRange = getOverlappingBucketRange(
        intersection.rect, defenderWorldBounds, defenderSubHurtBox.m_GridData, defenderFacingRight);

    for (int bucketY = bucketRange.startY; bucketY <= bucketRange.endY; bucketY++) {
        for (int bucketX = bucketRange.startX; bucketX <= bucketRange.endX; bucketX++) {
            neededBuckets[bucketY * defenderSubHurtBox.m_GridData.BucketMatrixWidth() + bucketX] =
                1;
        }
    }

    const std::vector<int> orderedBuckets = getOrderedBuckets(defenderSubHurtBox,
                                                              defenderWorldBounds,
                                                              intersection,
                                                              neededBuckets,
                                                              defenderFacingRight,
                                                              bucketRange.startX,
                                                              bucketRange.startY,
                                                              bucketRange.endX,
                                                              bucketRange.endY);

    if (defenderDebugData != nullptr)
        defenderDebugData->hurtBoxBounds.emplace_back(transformRectToWorldspace(
            defenderSubHurtBox.m_GridData.bounds, defenderSpriteRect, defenderFacingRight));
    // first check all outer pixels for overlaps, then also the inner ones
    for (int bucketIndex : orderedBuckets) {
        if (isInHitbox(attackPixels,
                       defenderSubHurtBox.m_OuterBuckets[bucketIndex],
                       defenderSubHurtBox.m_GridData.bounds,
                       defenderSpriteRect,
                       defenderFacingRight)) {
            return true;
        }
    }
    for (int bucketIndex : orderedBuckets) {
        if (isInHitbox(attackPixels,
                       defenderSubHurtBox.m_InnerBuckets[bucketIndex],
                       defenderSubHurtBox.m_GridData.bounds,
                       defenderSpriteRect,
                       defenderFacingRight)) {
            return true;
        }
    }

    return false;
}

}  // namespace

HitResult detectOverlap(const WorldHitBox& attackerHitBox,
                        const WorldHurtBox& defenderHurtBox,
                        PlayerCombatDebugData* attackerDebugData,
                        PlayerCombatDebugData* defenderDebugData) {
    const HitBox& attackerLocalHitBox = attackerHitBox.hitBox.get();
    const SDL_FRect attackerSpriteRect = attackerHitBox.spriteRect;
    const bool attackerFacingRight = attackerHitBox.facingRight;

    const HurtBox& defenderLocalHurtBox = defenderHurtBox.hurtBox.get();
    const SDL_FRect defenderSpriteRect = defenderHurtBox.spriteRect;
    const bool defenderFacingRight = defenderHurtBox.facingRight;
    // ok, so now I have WorldHitBox and WorldHurtBox
    // both contain the positions of the Hit and Hurtboxes, as well
    // as a FRect of where the general frame is in world coordinates and facingRight
    const DefinedHitbox definedHitbox = defineHitbox(attackerLocalHitBox,
                                                     attackerSpriteRect,
                                                     attackerFacingRight,
                                                     defenderLocalHurtBox,
                                                     defenderSpriteRect,
                                                     defenderFacingRight);
    if (attackerDebugData != nullptr)
        // do I want the whole sprite too?
        // attackerDebugData->hitBoxBounds.push_back(attackerSpriteRect);
        for (const auto& [value, rect] : definedHitbox.bucketRectsByHurtValue) {
            (void)value;
            attackerDebugData->hitBoxBounds.push_back(rect);
        }

    const auto& attackPixels = definedHitbox.attackPixels;
    const auto& bucketRectsByHurtValue = definedHitbox.bucketRectsByHurtValue;
    (void)bucketRectsByHurtValue;

    // so Hitbox is now fully defined:
    // next step: now I kind of want all
    for (int i = 3; i > 0; i--) {
        auto hurtIt = defenderLocalHurtBox.m_SubHurtBoxes.find(i);
        if (hurtIt == defenderLocalHurtBox.m_SubHurtBoxes.end()) {
            continue;
        }

        auto rectIt = bucketRectsByHurtValue.find(i);
        if (rectIt == bucketRectsByHurtValue.end()) {
            continue;
        }

        const SubHurtBox& subHurtBox = hurtIt->second;
        const SDL_FRect& hitboxRect = rectIt->second;

        if (checkIfHurtBoxWasHit(attackPixels,
                                 hitboxRect,
                                 subHurtBox,
                                 defenderSpriteRect,
                                 defenderFacingRight,
                                 defenderDebugData)) {
            return HitResult{.hit = true, .bestValue = i};
        }
    }

    return HitResult{};
}

}  // namespace sop
