#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

struct PushState {
    bool hitWallLeft = false;
    bool hitWallRight = false;
    bool hitPlayerLeft = false;
    bool hitPlayerRight = false;
    bool hitCeiling = false;
    bool hitGround = false;
};

struct CollisionBody {
    SDL_FRect Rect{};
    PushState Push{};
    bool Dynamic = false;
    float Weight = 1.0f;
};

struct CollisionSolveConfig {
    float Epsilon = 0.00001f;
    float VerticalSnapRatio = 0.1f;
};

struct CollisionSolveResult {
    bool Collided = false;
    bool HitFloor = false;
    bool HitCeiling = false;
    bool HitWallOnLeft = false;
    bool HitWallOnRight = false;
    SDL_FPoint Push{0.0f, 0.0f};
};

[[nodiscard]] CollisionSolveResult pushBoxes(
    CollisionBody& player,
    const SDL_FRect& platformRect,
    float verticalVelocity,
    const CollisionSolveConfig& config = {});

[[nodiscard]] CollisionSolveResult pushBoxes(
    CollisionBody& player1,
    CollisionBody& player2,
    float player1VerticalVelocity,
    float player2VerticalVelocity,
    const CollisionSolveConfig& config = {});

void ResetPushState(CollisionBody& body);

}  // namespace sop
