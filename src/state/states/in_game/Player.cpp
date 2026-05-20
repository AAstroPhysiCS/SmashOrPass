#include "smashorpass/state/states/in_game/Player.hpp"

#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/util.hpp"

namespace sop {

constexpr float kPlayerScale = 0.4f;
constexpr float kGravity = 0.28f;
constexpr float kJumpVelocity = -10.0f;
constexpr float kGroundProbeDistance = 5.0f;
constexpr float kWalkSpeed = 5.0f;
constexpr int kDashTicks = 20;
constexpr int kDashCooldownTicks = 54;
constexpr float kDashSpeed = 11.0f;

[[nodiscard]] bool IsActionHeld(const Input& input,
                                const InputTranslationHelper<InputAction>& translation,
                                InputAction action) {
    const auto* keys = translation.GetKeysForAction(action);
    if (keys == nullptr) {
        return false;
    }

    for (SDL_Keycode key : *keys) {
        if (input.GetKeyPressInfo(key)) {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool HasQueuedAction(const std::vector<InputAction>& inputQueue, InputAction action) {
    for (const InputAction queuedAction : inputQueue) {
        if (queuedAction == action) {
            return true;
        }
    }

    return false;
}

[[nodiscard]] SDL_FPoint SmallestPushOutOf(const SDL_FRect& player, const SDL_FRect& solid) {
    const float pushLeft = solid.x - (player.x + player.w);
    const float pushRight = solid.x + solid.w - player.x;
    const float pushUp = solid.y - (player.y + player.h);
    const float pushDown = solid.y + solid.h - player.y;

    const float horizontalPush = std::abs(pushLeft) < std::abs(pushRight) ? pushLeft : pushRight;
    const float verticalPush = std::abs(pushUp) < std::abs(pushDown) ? pushUp : pushDown;

    if (std::abs(verticalPush) <= std::abs(horizontalPush)) {
        return SDL_FPoint{.x = 0.0f, .y = verticalPush};
    }

    return SDL_FPoint{.x = horizontalPush, .y = 0.0f};
}

Player::Player(int playerId,
               CharacterAssetHandle asset,
               SDL_FPoint position,
               bool facingRight,
               float health,
               InputTranslationHelper<InputAction> inputTranslationHelper)
    : m_playerId(playerId),
      m_Asset(std::move(asset)),
      m_InputTranslationHelper(std::move(inputTranslationHelper)),
      m_Position(position),
      m_FacingRight(facingRight),
      m_State(PlayerState::IDLE),
      m_Health(health) {}

CharacterAnimation Player::GetAnimationToShow(AppCtx& ctx, const Arena& arena) const {
    switch (m_State) {
        case PlayerState::ATTACKING:
            return CharacterAnimation::Attacks;
        case PlayerState::DASHING:
            return CharacterAnimation::Dash;
        case PlayerState::IDLE:
        case PlayerState::RUNNING:
            break;
    }

    if (!IsOnGround(ctx, arena)) {
        return m_VelocityY < 0.0f ? CharacterAnimation::Ascending : CharacterAnimation::Falling;
    }

    switch (m_State) {
        case PlayerState::RUNNING:
            return CharacterAnimation::Walk;
        case PlayerState::IDLE:
        case PlayerState::ATTACKING:
        case PlayerState::DASHING:
            return CharacterAnimation::Idle;
    }

    return CharacterAnimation::Idle;
}

std::optional<SDL_FRect> Player::GetBaselineSpriteRect(
    const CharacterSpriteSheetFrame& frame) const {
    const SDL_FRect src = frame.m_Location;
    if (src.w <= 0.0f || src.h <= 0.0f) {
        return std::nullopt;
    }

    const float width = src.w * kPlayerScale;
    const float height = src.h * kPlayerScale;
    const float anchorX = static_cast<float>(frame.m_Anchor.x) * kPlayerScale;
    const float anchorY = static_cast<float>(frame.m_Anchor.y) * kPlayerScale;

    return SDL_FRect{
        .x = m_FacingRight ? m_Position.x - (width - anchorX) : m_Position.x - anchorX,
        .y = m_Position.y - anchorY,
        .w = width,
        .h = height,
    };
}

std::optional<SDL_FRect> Player::GetBaselineCollisionBox(AppCtx& ctx) const {
    auto asset = ctx.Assets.Get(m_Asset);
    if (!asset) {
        return std::nullopt;
    }

    const auto sheet = asset->get().m_SpriteSheets.find(m_CurrentAnimation);
    if (sheet == asset->get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        return std::nullopt;
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame) % frames.size()];
    const SDL_FRect collisionBox = frame.m_CollisionBox;
    if (collisionBox.w <= 0.0f || collisionBox.h <= 0.0f) {
        return std::nullopt;
    }

    std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return std::nullopt;
    }

    const float collisionX =
        m_FacingRight ? frame.m_Location.w - collisionBox.x - collisionBox.w : collisionBox.x;

    return SDL_FRect{
        .x = spriteRect->x + collisionX * kPlayerScale,
        .y = spriteRect->y + collisionBox.y * kPlayerScale,
        .w = collisionBox.w * kPlayerScale,
        .h = collisionBox.h * kPlayerScale,
    };
}

Result<void> Player::OnEvent(AppCtx& ctx, const Event& event) {
    (void)ctx;

    if (const auto* keyEvent = std::get_if<KeyEvent>(&event.Payload)) {
        if (keyEvent->Down && !keyEvent->Repeat) {
            if (std::optional<InputAction> action =
                    m_InputTranslationHelper.TranslateKey(keyEvent->Key)) {
                m_InputQueue.push_back(*action);
            }
        }
    }

    return Ok();
}

void Player::TickGameLogic(AppCtx& ctx, const Arena& arena) {
    // Apply user input
    const bool jumpRequested = HasQueuedAction(m_InputQueue, InputAction::JUMP);
    const bool dashRequested = HasQueuedAction(m_InputQueue, InputAction::DASH);
    const bool moveLeft =
        IsActionHeld(ctx.Input, m_InputTranslationHelper, InputAction::MOVE_LEFT);
    const bool moveRight =
        IsActionHeld(ctx.Input, m_InputTranslationHelper, InputAction::MOVE_RIGHT);
    const bool attackHeld =
        IsActionHeld(ctx.Input, m_InputTranslationHelper, InputAction::ATTACK);

    m_InputQueue.clear();

    if (m_DashCooldownTicksRemaining > 0) {
        --m_DashCooldownTicksRemaining;
    }

    const bool onGround = IsOnGround(ctx, arena);
    if (onGround) {
        m_AirDashAvailable = true;
        m_DashJumpAvailable = false;
    }

    if (dashRequested && m_DashTicksRemaining == 0 && m_DashCooldownTicksRemaining == 0 &&
        (onGround || m_AirDashAvailable)) {
        if (moveLeft != moveRight) {
            m_DashDirection = moveLeft ? -1.0f : 1.0f;
        } else {
            m_DashDirection = m_FacingRight ? 1.0f : -1.0f;
        }

        m_FacingRight = m_DashDirection > 0.0f;
        m_DashTicksRemaining = kDashTicks;
        m_DashCooldownTicksRemaining = kDashCooldownTicks;
        m_VelocityY = 0.0f;

        if (!onGround) {
            m_AirDashAvailable = false;
            m_DashJumpAvailable = true;
        }
    }

    if (m_DashTicksRemaining > 0) {
        m_Position.x += m_DashDirection * kDashSpeed;
        m_VelocityY = 0.0f;
        --m_DashTicksRemaining;
        m_State = PlayerState::DASHING;
    } else if (attackHeld) {
        m_State = PlayerState::ATTACKING;
    } else if (moveLeft != moveRight) {
        const float direction = moveLeft ? -1.0f : 1.0f;
        m_Position.x += direction * kWalkSpeed;
        m_FacingRight = direction > 0.0f;
        m_State = PlayerState::RUNNING;
    } else {
        m_State = PlayerState::IDLE;
    }

    if (m_State != PlayerState::DASHING && !attackHeld && jumpRequested) {
        if (onGround) {
            m_VelocityY = kJumpVelocity;
        } else if (m_DashJumpAvailable) {
            m_VelocityY = kJumpVelocity;
            m_DashJumpAvailable = false;
        }
    }

    // Apply gravity
    if (m_State != PlayerState::DASHING) {
        m_VelocityY += kGravity;
        m_Position.y += m_VelocityY;
    }

    if (!arena.asset) {
        return;
    }

    // Collision handling
    auto arenaAsset = ctx.Assets.Get(arena.asset);
    if (!arenaAsset) {
        return;
    }

    std::optional<SDL_FRect> playerCollisionBox = GetBaselineCollisionBox(ctx);
    if (!playerCollisionBox) {
        return;
    }

    const std::vector<SDL_FRect>& solidBoxes = arenaAsset->get().m_CollisionBoxes;
    const std::size_t maxPasses = solidBoxes.size() * 2;
    for (std::size_t pass = 0; pass < maxPasses; ++pass) {
        bool moved = false;

        for (const SDL_FRect& solidBox : solidBoxes) {
            if (!SDL_HasRectIntersectionFloat(&*playerCollisionBox, &solidBox)) {
                continue;
            }

            const SDL_FPoint push = SmallestPushOutOf(*playerCollisionBox, solidBox);
            m_Position.x += push.x;
            m_Position.y += push.y;
            playerCollisionBox->x += push.x;
            playerCollisionBox->y += push.y;

            if (push.y < 0.0f && m_VelocityY > 0.0f) {
                m_VelocityY = 0.0f;
            } else if (push.y > 0.0f && m_VelocityY < 0.0f) {
                m_VelocityY = 0.0f;
            }

            moved = true;
        }

        if (!moved) {
            break;
        }
    }

    if (IsOnGround(ctx, arena)) {
        m_AirDashAvailable = true;
        m_DashJumpAvailable = false;
    } else if (m_State == PlayerState::DASHING && m_AirDashAvailable) {
        m_DashJumpAvailable = true;
    }
}

void Player::TickAnimations(AppCtx& ctx, const Arena& arena) {
    auto asset = ctx.Assets.Get(m_Asset);
    if (!asset) {
        m_CurrentAnimationFrame = 0;
        return;
    }

    const CharacterAnimation animation = GetAnimationToShow(ctx, arena);
    if (animation != m_CurrentAnimation) {
        m_CurrentAnimationFrame = 0;
    }
    m_CurrentAnimation = animation;
    const auto sheet = asset->get().m_SpriteSheets.find(animation);
    if (sheet == asset->get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        m_CurrentAnimationFrame = 0;
        return;
    }

    m_CurrentAnimationFrame =
        (m_CurrentAnimationFrame + 1) % static_cast<int>(sheet->second.m_Frames.size());
}

bool Player::IsOnGround(AppCtx& ctx, const Arena& arena) const {
    if (!arena.asset) {
        return false;
    }

    auto arenaAsset = ctx.Assets.Get(arena.asset);
    if (!arenaAsset) {
        return false;
    }

    std::optional<SDL_FRect> playerCollisionBox = GetBaselineCollisionBox(ctx);
    if (!playerCollisionBox) {
        return false;
    }

    playerCollisionBox->y += kGroundProbeDistance;

    for (const SDL_FRect& solidBox : arenaAsset->get().m_CollisionBoxes) {
        if (SDL_HasRectIntersectionFloat(&*playerCollisionBox, &solidBox)) {
            return true;
        }
    }

    return false;
}

Result<void> Player::Render(AppCtx& ctx, const Arena& arena) const {
    TRY(asset, ctx.Assets.Get(m_Asset));

    const auto sheet = asset.get().m_SpriteSheets.find(m_CurrentAnimation);
    if (sheet == asset.get().m_SpriteSheets.end() || sheet->second.m_Frames.empty() ||
        sheet->second.m_Texture == nullptr) {
        return Ok();
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame) % frames.size()];
    const SDL_FRect src = frame.m_Location;
    std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return Ok();
    }

    TextureDrawParams params{};
    params.src = &src;
    params.dst = MapBaselineRectToArena(*spriteRect, arena.dimensions);
    params.flip = m_FacingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    return ctx.Renderer.DrawTexture(sheet->second.m_Texture.get(), params);
}

Result<void> Player::RenderCollisionBox(AppCtx& ctx, const Arena& arena) const {
    const std::optional<SDL_FRect> collisionBox = GetBaselineCollisionBox(ctx);
    if (!collisionBox) {
        return Ok();
    }

    return ctx.Renderer.DrawRect(MapBaselineRectToArena(*collisionBox, arena.dimensions),
                                   Color{255, 230, 0, 255});
}

}  // namespace sop
