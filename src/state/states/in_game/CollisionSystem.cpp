#include "smashorpass/state/states/in_game/CollisionSystem.hpp"

#include <algorithm>
#include <cmath>

namespace sop {

namespace {

[[nodiscard]] bool CanMoveLeft(const CollisionBody& body) {
    return !body.Contacts.hitWallLeft;
}

[[nodiscard]] bool CanMoveRight(const CollisionBody& body) {
    return !body.Contacts.hitWallRight;
}

void MoveBody(CollisionBody& body, const float dx, const float dy) {
    body.Rect.x += dx;
    body.Rect.y += dy;
}

[[nodiscard]] SDL_FPoint InvertOverlap(const SDL_FPoint overlap) {
    return SDL_FPoint{.x = -overlap.x, .y = -overlap.y};
}

[[nodiscard]] bool ShouldResolveAsFloor(const CollisionBody& player,
                                        const SDL_FPoint overlap,
                                        const float verticalVelocity,
                                        const float otherVerticalVelocity,
                                        const CollisionSolveConfig& config) {
    // Epsilon is to prevent some glitching (noticable when player stands on top of another player and the bottom player jumps)
    // push player up, if <10% of his body are inside the bottom collision box and he is moving downwards
    // If the lower body moves upward faster, push the top player up even if the top player is moving upward too.
    const float heightBasedSnap = player.Rect.h * config.VerticalSnapRatio;
    const float velocityBasedSnap =
        std::max(0.0f, verticalVelocity) * config.FloorSnapVelocityMultiplier;
    const float maxFloorSnap = std::max(heightBasedSnap, velocityBasedSnap);

    const bool overlapIsBelowPlayer = overlap.y > config.Epsilon;
    const bool shallowEnoughToSnap = overlap.y < maxFloorSnap;
    const bool playerIsFalling = verticalVelocity >= 0.0f;
    const bool lowerBodyIsMovingUpIntoPlayer = otherVerticalVelocity <= verticalVelocity;

    return overlapIsBelowPlayer && shallowEnoughToSnap &&
           (playerIsFalling || lowerBodyIsMovingUpIntoPlayer);
}

[[nodiscard]] bool ShouldResolveAsCeiling(const CollisionBody& player,
                                          const SDL_FPoint overlap,
                                          const float verticalVelocity,
                                          const CollisionSolveConfig& config) {
    // Epsilon is to prevent some glitching (noticable when player stands on top of another player and the bottom player jumps)
    // push player up, if <10% of his body are inside the top collision box and he is moving upwards
    const bool overlapIsAbovePlayer = overlap.y < -config.Epsilon;
    const bool shallowEnoughToSnap = overlap.y > player.Rect.h * -config.VerticalSnapRatio;
    const bool playerIsMovingUp = verticalVelocity <= 0.0f;

    return overlapIsAbovePlayer && shallowEnoughToSnap && playerIsMovingUp;
}

void pushPlayer(CollisionBody& player,
                const SDL_FPoint distance,
                const float factor,
                const bool pushY) {
    const float dx = distance.x * factor;
    const float dy = distance.y * factor;
    if (pushY) {
        MoveBody(player, 0.0f, dy);
    } else {
        MoveBody(player, dx, 0.0f);
    }
}

[[nodiscard]] SDL_FPoint getOverlap(const SDL_FRect& rect1, const SDL_FRect& rect2) {
    const float center1X = rect1.x + rect1.w * 0.5f;
    const float center1Y = rect1.y + rect1.h * 0.5f;
    const float center2X = rect2.x + rect2.w * 0.5f;
    const float center2Y = rect2.y + rect2.h * 0.5f;

    const float deltaX = center2X - center1X;
    const float deltaY = center2Y - center1Y;

    const float overlapX = (rect1.w + rect2.w) * 0.5f - std::abs(deltaX);
    const float overlapY = (rect1.h + rect2.h) * 0.5f - std::abs(deltaY);

    if (overlapX <= 0.0f || overlapY <= 0.0f) {
        return SDL_FPoint{.x = 0.0f, .y = 0.0f};
    }

    return SDL_FPoint{
        .x = deltaX >= 0.0f ? overlapX : -overlapX,
        .y = deltaY >= 0.0f ? overlapY : -overlapY,
    };
}

}  // namespace

bool ResolveCollision(CollisionBody& player,
                      const SDL_FRect& platformRect,
                      const float verticalVelocity,
                      const CollisionSolveConfig& config) {
    // between platform Boxes and Player Collision Boxes
    // get Overlap and then determine in which direction the player should be pushed
    const SDL_FPoint overlap = getOverlap(player.Rect, platformRect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return false;
    }

    if (ShouldResolveAsFloor(player, overlap, verticalVelocity, 0.0f, config)) {
        pushPlayer(player, overlap, -1.0f, true);
        player.Contacts.hitGround = true;
    } else if (ShouldResolveAsCeiling(player, overlap, verticalVelocity, config)) {
        pushPlayer(player, overlap, -1.0f, true);
        player.Contacts.hitCeiling = true;
    } else {
        const bool playerNeedsToMoveLeft = overlap.x > 0.0f;
        const bool playerCanMoveX = playerNeedsToMoveLeft ? CanMoveLeft(player)
                                                          : CanMoveRight(player);
        if (!playerCanMoveX) {
            return false;
        }

        pushPlayer(player, overlap, -1.0f, false);
        if (overlap.x > 0.0f) {
            player.Contacts.hitWallRight = true;
        } else {
            player.Contacts.hitWallLeft = true;
        }
    }

    return true;
}

bool ResolveCollision(CollisionBody& player1,
                      CollisionBody& player2,
                      const float player1VerticalVelocity,
                      const float player2VerticalVelocity,
                      const CollisionSolveConfig& config) {
    // between 2 Player Collision Boxes
    // get Overlap and then determine in which direction the player should be pushed
    const SDL_FPoint overlap = getOverlap(player1.Rect, player2.Rect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return false;
    }

    // when one player stands on top of the other, only the top one is pushed up, the bottom player is not pushed down
    // when players are next to each other they are both pushed out
    if (ShouldResolveAsFloor(player1, overlap, player1VerticalVelocity, player2VerticalVelocity, config)) {
        pushPlayer(player1, overlap, -1.0f, true);
        player1.Contacts.hitGround = true;
    } else if (ShouldResolveAsFloor(player2, InvertOverlap(overlap), player2VerticalVelocity, player1VerticalVelocity, config)) {
        pushPlayer(player2, overlap, 1.0f, true);
        player2.Contacts.hitGround = true;
    } else {
        const bool player1NeedsToMoveLeft = overlap.x > 0.0f;
        const bool player1CanMoveX =
            player1NeedsToMoveLeft ? CanMoveLeft(player1) : CanMoveRight(player1);
        const bool player2CanMoveX =
            player1NeedsToMoveLeft ? CanMoveRight(player2) : CanMoveLeft(player2);
        if (!player1CanMoveX && !player2CanMoveX) {
            return false;
        }

        if (player1CanMoveX && player2CanMoveX) {
            const float totalWeight = player1.Weight + player2.Weight;
            if (totalWeight <= 0.0f) {
                pushPlayer(player1, overlap, -0.5f, false);
                pushPlayer(player2, overlap, 0.5f, false);
            } else {
                const float player1PushFactor = -player2.Weight / totalWeight;
                const float player2PushFactor = player1.Weight / totalWeight;
                pushPlayer(player1, overlap, player1PushFactor, false);
                pushPlayer(player2, overlap, player2PushFactor, false);
            }
        } else if (player1CanMoveX) {
            pushPlayer(player1, overlap, -1.0f, false);
        } else {
            pushPlayer(player2, overlap, 1.0f, false);
        }

        if (player1NeedsToMoveLeft) {
            player1.Contacts.hitPlayerRight = true;
            player2.Contacts.hitPlayerLeft = true;
        } else {
            player1.Contacts.hitPlayerLeft = true;
            player2.Contacts.hitPlayerRight = true;
        }
    }

    return true;
}

void ResetCollisionContacts(CollisionBody& body) {
    body.Contacts = CollisionContacts{};
}

}  // namespace sop
