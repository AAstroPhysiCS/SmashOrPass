#include "smashorpass/asset/assets/CharacterCombatData.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <utility>

namespace sop {

namespace {

constexpr int kBucketReorderStride = 5;

void reorderBucketByStride(std::vector<SDL_Point>& bucket, int stride) {
    // Points in each Bucket Vector are reordered, so you can
    // check Pixels from different Areas earlier and hopefully find an overlap quicker
    if (bucket.size() <= 1 || stride <= 1) {
        return;
    }

    std::vector<SDL_Point> reordered;
    reordered.reserve(bucket.size());

    for (int offset = 0; offset < stride; offset++) {
        for (int index = offset; index < static_cast<int>(bucket.size()); index += stride) {
            reordered.push_back(bucket[index]);
        }
    }

    bucket = std::move(reordered);
}

void reorderBucketsByStride(std::vector<std::vector<SDL_Point>>& buckets, int stride) {
    for (std::vector<SDL_Point>& bucket : buckets) {
        reorderBucketByStride(bucket, stride);
    }
}

bool isOuterPixel(const ChannelPlane& channel, int x, int y, int channelValue) {
    const int width = static_cast<int>(channel.size());
    const int height = static_cast<int>(channel[0].size());

    const int neighbors[4][2] = {
        {-1, 0},
        {1, 0},
        {0, -1},
        {0, 1},
    };

    for (const auto& neighbor : neighbors) {
        const int nextX = x + neighbor[0];
        const int nextY = y + neighbor[1];

        if (nextX < 0 || nextX >= width || nextY < 0 || nextY >= height) {
            return true;
        }
        if (channel[nextX][nextY] != channelValue) {
            return true;
        }
    }

    return false;
}

std::pair<std::vector<std::vector<SDL_Point>>, std::vector<std::vector<SDL_Point>>>
createInnerOuterBuckets(const ChannelPlane& channel,
                        const SDL_FRect& boundingBox,
                        int gridSize,
                        int bucketMatrixWidth,
                        int bucketMatrixHeight,
                        int channelValue) {
    // Outer buckets store hurtbox edge pixels, split by grid cell.
    // With continuous Hitboxes this should suffice to find an overlap -> less computation
    std::vector<std::vector<SDL_Point>> innerBuckets(bucketMatrixWidth * bucketMatrixHeight);
    std::vector<std::vector<SDL_Point>> outerBuckets(bucketMatrixWidth * bucketMatrixHeight);

    const int left = static_cast<int>(boundingBox.x);
    const int top = static_cast<int>(boundingBox.y);
    const int right = left + static_cast<int>(boundingBox.w) - 1;
    const int bottom = top + static_cast<int>(boundingBox.h) - 1;

    for (int x = left; x <= right; x++) {
        for (int y = top; y <= bottom; y++) {
            if (channel[x][y] != channelValue) {
                continue;
            }

            const int localX = x - left;
            const int localY = y - top;

            int cellX = localX / gridSize;
            int cellY = localY / gridSize;

            if (cellX >= bucketMatrixWidth)
                cellX = bucketMatrixWidth - 1;
            if (cellY >= bucketMatrixHeight)
                cellY = bucketMatrixHeight - 1;

            const int bucketIndex = cellY * bucketMatrixWidth + cellX;
            if (isOuterPixel(channel, x, y, channelValue)) {
                outerBuckets[bucketIndex].push_back(SDL_Point{localX, localY});
            } else {
                innerBuckets[bucketIndex].push_back(SDL_Point{localX, localY});
            }
        }
    }

    reorderBucketsByStride(innerBuckets, kBucketReorderStride);
    reorderBucketsByStride(outerBuckets, kBucketReorderStride);

    return {innerBuckets, outerBuckets};
}

SDL_FRect getBounds(const ChannelPlane& channel, int channelValue) {
    if (channel.empty() || channel[0].empty()) {
        return SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
    }

    const int width = static_cast<int>(channel.size());
    const int height = static_cast<int>(channel[0].size());

    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (channel[x][y] != channelValue) {
                continue;
            }

            if (x < minX)
                minX = x;
            if (y < minY)
                minY = y;
            if (x > maxX)
                maxX = x;
            if (y > maxY)
                maxY = y;
        }
    }

    if (maxX == -1) {
        return SDL_FRect{0.0f, 0.0f, 0.0f, 0.0f};
    }

    return SDL_FRect{static_cast<float>(minX),
                     static_cast<float>(minY),
                     static_cast<float>(maxX - minX + 1),
                     static_cast<float>(maxY - minY + 1)};
}

std::vector<SDL_Point> createMatrix(int matrixHeight,
                                    int matrixWidth,
                                    int gridSize,
                                    const SDL_FRect& boundingBox) {
    std::vector<SDL_Point> matrix(matrixHeight * matrixWidth);

    const int maxX = static_cast<int>(boundingBox.w) - 1;
    const int maxY = static_cast<int>(boundingBox.h) - 1;

    for (int y = 0; y < matrixHeight; y++) {
        for (int x = 0; x < matrixWidth; x++) {
            int pointX = x * gridSize;
            int pointY = y * gridSize;

            if (pointX > maxX)
                pointX = maxX;
            if (pointY > maxY)
                pointY = maxY;

            matrix[y * matrixWidth + x] = SDL_Point{pointX, pointY};
        }
    }

    return matrix;
}

std::vector<std::vector<SDL_Point>> createBuckets(const ChannelPlane& channel,
                                                  const SDL_FRect& boundingBox,
                                                  int gridSize,
                                                  int bucketMatrixWidth,
                                                  int bucketMatrixHeight,
                                                  int channelValue) {
    // Each Bucket is one Grid Cell.
    // We later compute which Grid Cells overlap and only compare the pixels in those Buckets
    std::vector<std::vector<SDL_Point>> buckets(bucketMatrixWidth * bucketMatrixHeight);

    const int left = static_cast<int>(boundingBox.x);
    const int top = static_cast<int>(boundingBox.y);
    const int right = left + static_cast<int>(boundingBox.w) - 1;
    const int bottom = top + static_cast<int>(boundingBox.h) - 1;

    for (int x = left; x <= right; x++) {
        for (int y = top; y <= bottom; y++) {
            if (channel[x][y] != channelValue) {
                continue;
            }

            const int localX = x - left;
            const int localY = y - top;

            int cellX = localX / gridSize;
            int cellY = localY / gridSize;

            if (cellX >= bucketMatrixWidth)
                cellX = bucketMatrixWidth - 1;
            if (cellY >= bucketMatrixHeight)
                cellY = bucketMatrixHeight - 1;

            const int bucketIndex = cellY * bucketMatrixWidth + cellX;
            buckets[bucketIndex].push_back(SDL_Point{localX, localY});
        }
    }

    return buckets;
}

SubHurtBox setupSubHurtBox(const ChannelPlane& blueChannel,
                           int targetGridSize,
                           int blueChannelValue) {
    // SubHurtBox -> All pixels with similar blue (=hurt) values -> head, torso etc.
    const SDL_FRect boundingBox = getBounds(blueChannel, blueChannelValue);

    const int gridSize = std::max(1, targetGridSize);
    const int matrixHeight = (static_cast<int>(boundingBox.h) - 1) / gridSize + 2;
    const int matrixWidth = (static_cast<int>(boundingBox.w) - 1) / gridSize + 2;
    const int bucketMatrixHeight = matrixHeight - 1;
    const int bucketMatrixWidth = matrixWidth - 1;

    if (boundingBox.w <= 0.0f || boundingBox.h <= 0.0f) {
        return SubHurtBox{
            .m_GridData =
                GridData{
                    .bounds = boundingBox,
                    .matrixWidth = 0,
                    .matrixHeight = 0,
                    .cellSize = gridSize,
                    .m_GridMatrix = {},
                },
            .m_InnerBuckets = {},
            .m_OuterBuckets = {},
        };
    }

    std::vector<SDL_Point> matrix = createMatrix(matrixHeight, matrixWidth, gridSize, boundingBox);

    auto [innerBuckets, outerBuckets] = createInnerOuterBuckets(blueChannel,
                                                                boundingBox,
                                                                gridSize,
                                                                bucketMatrixWidth,
                                                                bucketMatrixHeight,
                                                                blueChannelValue);

    return SubHurtBox{
        .m_GridData =
            GridData{
                .bounds = boundingBox,
                .matrixWidth = matrixWidth,
                .matrixHeight = matrixHeight,
                .cellSize = gridSize,
                .m_GridMatrix = std::move(matrix),
            },
        .m_InnerBuckets = std::move(innerBuckets),
        .m_OuterBuckets = std::move(outerBuckets),
    };
}

}  // namespace

HitBox setupHitbox(const ChannelPlane& redChannel, int targetGridSize, AttackData attackData) {
    // build a Hitbox Grid, with one vector of Hitbox pixels per Cell
    const SDL_FRect boundingBox = getBounds(redChannel, 1);

    const int gridSize = std::max(1, targetGridSize);
    const int matrixHeight = (static_cast<int>(boundingBox.h) - 1) / gridSize + 2;
    const int matrixWidth = (static_cast<int>(boundingBox.w) - 1) / gridSize + 2;
    const int bucketMatrixHeight = matrixHeight - 1;
    const int bucketMatrixWidth = matrixWidth - 1;

    if (boundingBox.w <= 0.0f || boundingBox.h <= 0.0f) {
        return HitBox{
            .m_GridData =
                GridData{
                    .bounds = boundingBox,
                    .matrixWidth = 0,
                    .matrixHeight = 0,
                    .cellSize = gridSize,
                    .m_GridMatrix = {},
                },
            .m_Buckets = {},
            .m_AttackData = attackData,
        };
    }

    std::vector<SDL_Point> matrix = createMatrix(matrixHeight, matrixWidth, gridSize, boundingBox);

    std::vector<std::vector<SDL_Point>> buckets =
        createBuckets(redChannel, boundingBox, gridSize, bucketMatrixWidth, bucketMatrixHeight, 1);

    return HitBox{
        .m_GridData =
            GridData{
                .bounds = boundingBox,
                .matrixWidth = matrixWidth,
                .matrixHeight = matrixHeight,
                .cellSize = gridSize,
                .m_GridMatrix = std::move(matrix),
            },
        .m_Buckets = std::move(buckets),
        .m_AttackData = attackData,
    };
}

AttackData loadAttackData(const nlohmann::json& frameJson) {
    // Get Info about attack from each frame (damage, knockback, stun)
    const auto attackIt = frameJson.find("attack");
    if (attackIt == frameJson.end() || attackIt->is_null()) {
        return AttackData{};
    }

    const nlohmann::json& attackJson = *attackIt;
    const nlohmann::json& knockbackJson = attackJson.at("knockback");

    return AttackData{
        .m_Id = attackJson.at("id").get<int>(),
        .m_Damage = attackJson.at("damage").get<float>(),
        .m_Knockback =
            SDL_FPoint{
                .x = knockbackJson.at("x").get<float>(),
                .y = knockbackJson.at("y").get<float>(),
            },
        .m_HitstunTicks = attackJson.at("hitstunTicks").get<int>(),
        .m_HitCooldownTicks = attackJson.at("hitCooldownTicks").get<int>(),
    };
}

HurtBox setupHurtBox(const ChannelPlane& blueChannel, int targetGridSize) {
    // gets values 1-3 for Blue Channel, 3 = most vulnerable (head) etc.
    // builds one SubHurtBox per Value, later we first check 3 for a hit, then 2 etc.
    HurtBox hurtBox;

    for (int value = 1; value <= 3; value++) {
        const SubHurtBox subHurtBox = setupSubHurtBox(blueChannel, targetGridSize, value);
        if (subHurtBox.m_GridData.bounds.w == 0.0f || subHurtBox.m_GridData.bounds.h == 0.0f) {
            continue;
        }

        hurtBox.m_SubHurtBoxes[value] = subHurtBox;
    }

    return hurtBox;
}

}  // namespace sop
