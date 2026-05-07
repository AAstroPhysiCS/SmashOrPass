#include "smashorpass/core/PlayerSpritePlacement.hpp"

#include "smashorpass/core/Base.hpp"

namespace sop::detail {

PlayerSpritePlacement MakePlayerSpritePlacement(SDL_FPoint anchorPosition,
                                                const SpriteSheetFrame& frame,
                                                bool facingRight,
                                                float scale) {
    SOP_ASSERT(frame.x_right > frame.x_left, "Sprite frame must have positive width");
    SOP_ASSERT(frame.y_bottom > frame.y_top, "Sprite frame must have positive height");
    SOP_ASSERT(scale >= 0.0f, "Sprite placement requires a non-negative scale");

    const float frameWidth = static_cast<float>(frame.x_right - frame.x_left);
    const float frameHeight = static_cast<float>(frame.y_bottom - frame.y_top);
    const float destinationWidth = frameWidth * scale;
    const float destinationHeight = frameHeight * scale;
    const SDL_FPoint origin{static_cast<float>(frame.anchor_x) * scale,
                            static_cast<float>(frame.anchor_y) * scale};
    const float destinationX = facingRight ? anchorPosition.x - (destinationWidth - origin.x)
                                           : anchorPosition.x - origin.x;

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
                destinationX,
                anchorPosition.y - origin.y,
                destinationWidth,
                destinationHeight,
            },
        .Origin = origin,
        .Flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE,
    };
}

}  // namespace sop::detail
