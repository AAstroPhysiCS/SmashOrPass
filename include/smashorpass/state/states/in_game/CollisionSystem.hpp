#pragma once

#include <SDL3/SDL_rect.h>

namespace sop {

struct CollisionContacts {
    bool hitWallLeft = false;
    bool hitWallRight = false;
    bool hitPlayerLeft = false;
    bool hitPlayerRight = false;
    bool hitCeiling = false;
    bool hitGround = false;
};

struct CollisionBody {
    SDL_FRect Rect{};
    CollisionContacts Contacts{};
    bool Dynamic = false;
    float Weight = 1.0f;
};

struct CollisionSolveConfig {
    float Epsilon = 0.00001f;
    float VerticalSnapRatio = 0.1f;
    float FloorSnapVelocityMultiplier = 1.1f;
};

[[nodiscard]] bool ResolveCollision(
    CollisionBody& player,
    const SDL_FRect& platformRect,
    float verticalVelocity,
    const CollisionSolveConfig& config = {});

[[nodiscard]] bool ResolveCollision(
    CollisionBody& player1,
    CollisionBody& player2,
    float player1VerticalVelocity,
    float player2VerticalVelocity,
    const CollisionSolveConfig& config = {});

void ResetCollisionContacts(CollisionBody& body);

}  // namespace sop
