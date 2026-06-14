#include "smashorpass/state/states/in_game/CollisionSystem.hpp"

#include <cmath>

namespace sop {

namespace {

[[nodiscard]] bool CanMoveLeft(const CollisionBody& body) {
    return !body.Push.hitWallLeft;
}

[[nodiscard]] bool CanMoveRight(const CollisionBody& body) {
    return !body.Push.hitWallRight;
}

void MoveBody(CollisionBody& body, float dx, float dy) {
    body.Rect.x += dx;
    body.Rect.y += dy;
}

void MarkPushResult(CollisionSolveResult& result, SDL_FPoint push, float epsilon) {
    result.Collided = true;
    result.Push = push;

    if (push.y < -epsilon) {
        result.HitFloor = true;
    } else if (push.y > epsilon) {
        result.HitCeiling = true;
    } else if (push.x > epsilon) {
        result.HitWallOnLeft = true;
    } else if (push.x < -epsilon) {
        result.HitWallOnRight = true;
    }
}

[[nodiscard]] SDL_FPoint InvertOverlap(SDL_FPoint overlap) {
    return SDL_FPoint{.x = -overlap.x, .y = -overlap.y};
}

[[nodiscard]] bool ShouldResolveAsFloor(const CollisionBody& player,
                                        SDL_FPoint overlap,
                                        float verticalVelocity,
                                        const CollisionSolveConfig& config) {
    // Epsilon is to prevent some glitching (noticable when player stands on top of another player and the bottom player jumps)
    // push player up, if <10% of his body are inside the bottom collision box and he is moving downwards
    return overlap.y > config.Epsilon && !player.Push.hitGround &&
           overlap.y < player.Rect.h * config.VerticalSnapRatio && verticalVelocity >= 0.0f;
}

[[nodiscard]] bool ShouldResolveAsCeiling(const CollisionBody& player,
                                          SDL_FPoint overlap,
                                          float verticalVelocity,
                                          const CollisionSolveConfig& config) {
    // Epsilon is to prevent some glitching (noticable when player stands on top of another player and the bottom player jumps)
    // push player up, if <10% of his body are inside the top collision box and he is moving upwards
    return overlap.y < -config.Epsilon && !player.Push.hitCeiling &&
           overlap.y > player.Rect.h * -config.VerticalSnapRatio && verticalVelocity <= 0.0f;
}

void pushPlayer(CollisionBody& player,
                SDL_FPoint distance,
                float factor,
                bool pushY,
                CollisionSolveResult& result,
                const CollisionSolveConfig& config) {
    const float dx = distance.x * factor;
    const float dy = distance.y * factor;
    if (pushY) {
        MoveBody(player, 0.0f, dy);
        MarkPushResult(result, SDL_FPoint{.x = 0.0f, .y = dy}, config.Epsilon);
    } else {
        MoveBody(player, dx, 0.0f);
        MarkPushResult(result, SDL_FPoint{.x = dx, .y = 0.0f}, config.Epsilon);
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

CollisionSolveResult pushBoxes(CollisionBody& player,
                               const SDL_FRect& platformRect,
                               float verticalVelocity,
                               const CollisionSolveConfig& config) {
    // between platform Boxes and Player Collision Boxes
    // get Overlap and then determine in which direction the player should be pushed
    CollisionSolveResult result{};
    const SDL_FPoint overlap = getOverlap(player.Rect, platformRect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return result;
    }

    if (ShouldResolveAsFloor(player, overlap, verticalVelocity, config)) {
        pushPlayer(player, overlap, -1.0f, true, result, config);
        player.Push.hitGround = true;
    } else if (ShouldResolveAsCeiling(player, overlap, verticalVelocity, config)) {
        pushPlayer(player, overlap, -1.0f, true, result, config);
        player.Push.hitCeiling = true;
    } else {
        const bool playerNeedsToMoveLeft = overlap.x > 0.0f;
        const bool playerCanMoveX = playerNeedsToMoveLeft ? CanMoveLeft(player)
                                                          : CanMoveRight(player);
        if (!playerCanMoveX) {
            return result;
        }

        pushPlayer(player, overlap, -1.0f, false, result, config);
        if (overlap.x > 0.0f) {
            player.Push.hitWallRight = true;
        } else {
            player.Push.hitWallLeft = true;
        }
    }

    return result;
}

CollisionSolveResult pushBoxes(CollisionBody& player1,
                               CollisionBody& player2,
                               float player1VerticalVelocity,
                               float player2VerticalVelocity,
                               const CollisionSolveConfig& config) {
    // between 2 Player Collision Boxes
    // get Overlap and then determine in which direction the player should be pushed
    CollisionSolveResult result{};
    const SDL_FPoint overlap = getOverlap(player1.Rect, player2.Rect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return result;
    }

    // when one player stands on top of the other, only the top one is pushed up, the bottom player is not pushed down
    // when players are next to each other they are both pushed out
    if (ShouldResolveAsFloor(player1, overlap, player1VerticalVelocity, config)) {
        pushPlayer(player1, overlap, -1.0f, true, result, config);
        player1.Push.hitGround = true;
    } else if (ShouldResolveAsFloor(player2, InvertOverlap(overlap), player2VerticalVelocity, config)) {
        pushPlayer(player2, overlap, 1.0f, true, result, config);
        player2.Push.hitGround = true;
    } else {
        const bool player1NeedsToMoveLeft = overlap.x > 0.0f;
        const bool player1CanMoveX =
            player1NeedsToMoveLeft ? CanMoveLeft(player1) : CanMoveRight(player1);
        const bool player2CanMoveX =
            player1NeedsToMoveLeft ? CanMoveRight(player2) : CanMoveLeft(player2);
        if (!player1CanMoveX && !player2CanMoveX) {
            return result;
        }

        if (player1CanMoveX && player2CanMoveX) {
            const float totalWeight = player1.Weight + player2.Weight;
            if (totalWeight <= 0.0f) {
                pushPlayer(player1, overlap, -0.5f, false, result, config);
                pushPlayer(player2, overlap, 0.5f, false, result, config);
            } else {
                const float player1PushFactor = -player2.Weight / totalWeight;
                const float player2PushFactor = player1.Weight / totalWeight;
                pushPlayer(player1, overlap, player1PushFactor, false, result, config);
                pushPlayer(player2, overlap, player2PushFactor, false, result, config);
            }
        } else if (player1CanMoveX) {
            pushPlayer(player1, overlap, -1.0f, false, result, config);
        } else {
            pushPlayer(player2, overlap, 1.0f, false, result, config);
        }

        if (player1NeedsToMoveLeft) {
            player1.Push.hitPlayerRight = true;
            player2.Push.hitPlayerLeft = true;
        } else {
            player1.Push.hitPlayerLeft = true;
            player2.Push.hitPlayerRight = true;
        }
    }

    return result;
}

void ResetPushState(CollisionBody& body) {
    body.Push = PushState{};
}

}  // namespace sop
