#include "PlayerSpritePlacement.hpp"

#include "smashorpass/core/Base.hpp"

namespace sop::detail {

PlayerSpritePlacement MakePlayerSpritePlacement(const SDL_FRect& placeholderRect,
                                                const SpriteSheetFrame& frame,
                                                bool facingRight,
                                                float placeholderHeight) {
    SOP_ASSERT(frame.x_right > frame.x_left, "Sprite frame must have positive width");
    SOP_ASSERT(frame.y_bottom > frame.y_top, "Sprite frame must have positive height");
    SOP_ASSERT(frame.source_w > 0, "Sprite frame must have a positive source width");
    SOP_ASSERT(frame.source_h > 0, "Sprite frame must have a positive source height");
    SOP_ASSERT(frame.center_x >= frame.x_left && frame.center_x < frame.x_right,
               "Sprite frame center_x must lie inside the frame bounds");
    SOP_ASSERT(frame.center_y >= frame.y_top && frame.center_y < frame.y_bottom,
               "Sprite frame center_y must lie inside the frame bounds");

    const float frameWidth = static_cast<float>(frame.x_right - frame.x_left);
    const float frameHeight = static_cast<float>(frame.y_bottom - frame.y_top);
    const float scale = placeholderHeight / static_cast<float>(frame.source_h);
    const float destinationWidth = static_cast<float>(frame.source_w) * scale;
    const float destinationHeight = static_cast<float>(frame.source_h) * scale;
    const float anchorX = placeholderRect.x + (placeholderRect.w * 0.5f);
    const float anchorY = placeholderRect.y + (placeholderRect.h * 0.5f);
    const float localCenterX = static_cast<float>(frame.center_x - frame.x_left);
    const float localCenterY = static_cast<float>(frame.center_y - frame.y_top);
    const SDL_FPoint origin{localCenterX * scale, localCenterY * scale};

    return PlayerSpritePlacement{
        .SourceRect =
            SDL_FRect{
                static_cast<float>(frame.x_left),
                static_cast<float>(frame.y_top),
                frameWidth,
                frameHeight,
            },
        .DestinationRect =
            SDL_FRect{
                anchorX - origin.x,
                anchorY - origin.y,
                destinationWidth,
                destinationHeight,
            },
        .Origin = origin,
        .Flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL,
    };
}

}  // namespace sop::detail
