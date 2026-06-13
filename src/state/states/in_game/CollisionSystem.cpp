#include "smashorpass/state/states/in_game/CollisionSystem.hpp"

#include <cmath>

namespace sop {

namespace {

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

[[nodiscard]] bool ShouldResolveAsFloor(const CollisionBody& player,
                                        SDL_FPoint overlap,
                                        float verticalVelocity,
                                        const CollisionSolveConfig& config) {
    return overlap.y > config.Epsilon && player.Push.canPushUp &&
           overlap.y < player.Rect.h * config.VerticalSnapRatio && verticalVelocity >= 0.0f;
}

[[nodiscard]] bool ShouldResolveAsCeiling(const CollisionBody& player,
                                          SDL_FPoint overlap,
                                          float verticalVelocity,
                                          const CollisionSolveConfig& config) {
    (void)player;
    return overlap.y < -config.Epsilon &&
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

}  // namespace

SDL_FPoint getOverlap(const SDL_FRect& rect1, const SDL_FRect& rect2) {
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

CollisionSolveResult pushBoxes(CollisionBody& player,
                               const SDL_FRect& platformRect,
                               float verticalVelocity,
                               const CollisionSolveConfig& config) {
    CollisionSolveResult result{};
    const SDL_FPoint overlap = getOverlap(player.Rect, platformRect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return result;
    }

    if (ShouldResolveAsFloor(player, overlap, verticalVelocity, config)) {
        pushPlayer(player, overlap, -1.0f, true, result, config);
    } else if (ShouldResolveAsCeiling(player, overlap, verticalVelocity, config)) {
        pushPlayer(player, overlap, -1.0f, true, result, config);
    } else {
        const bool playerNeedsToMoveLeft = overlap.x > 0.0f;
        const bool playerCanMoveX = playerNeedsToMoveLeft ? player.Push.canPushLeft
                                                          : player.Push.canPushRight;
        if (!playerCanMoveX) {
            return result;
        }

        pushPlayer(player, overlap, -1.0f, false, result, config);
        if (overlap.x > 0.0f) {
            player.Push.canPushRight = false;
        } else {
            player.Push.canPushLeft = false;
        }
    }

    return result;
}

CollisionSolveResult pushBoxes(CollisionBody& player1,
                               CollisionBody& player2,
                               float player1VerticalVelocity,
                               float player2VerticalVelocity,
                               const CollisionSolveConfig& config) {
    CollisionSolveResult result{};
    const SDL_FPoint overlap = getOverlap(player1.Rect, player2.Rect);
    if (overlap.x == 0.0f && overlap.y == 0.0f) {
        return result;
    }

    if (ShouldResolveAsFloor(player1, overlap, player1VerticalVelocity, config)) {
        pushPlayer(player1, overlap, -1.0f, true, result, config);
    } else if (overlap.y < -config.Epsilon && player2.Push.canPushUp &&
               overlap.y > player2.Rect.h * -config.VerticalSnapRatio &&
               player2VerticalVelocity >= 0.0f) {
        pushPlayer(player2, overlap, 1.0f, true, result, config);
    } else {
        const bool player1NeedsToMoveLeft = overlap.x > 0.0f;
        const bool player1CanMoveX =
            player1NeedsToMoveLeft ? player1.Push.canPushLeft : player1.Push.canPushRight;
        const bool player2CanMoveX =
            player1NeedsToMoveLeft ? player2.Push.canPushRight : player2.Push.canPushLeft;
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
    }

    return result;
}

void ResetPushState(CollisionBody& body) {
    body.Push = PushState{};
}

}  // namespace sop
