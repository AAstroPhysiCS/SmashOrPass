#include "smashorpass/state/states/in_game/Player.hpp"

#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

#include "smashorpass/asset/effects/CharacterFrameEffectMask.hpp"
#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/util.hpp"

namespace sop {

constexpr float kPlayerScale = 0.4f;
constexpr float kGravity = 0.28f;
constexpr float kMaxFallSpeed = 20.0f;
constexpr float kJumpVelocity = -10.0f;
constexpr float kGroundProbeDistance = 5.0f;
constexpr float kWalkSpeed = 5.0f;
constexpr float kWalkAcceleration = 1.0f;
constexpr float kAirMovementFactor = 0.2f;
constexpr float kGroundFriction = 0.1f;
constexpr float kAirFriction = 0.04f;
constexpr int kDashTicks = 20;
constexpr int kDashCooldownTicks = 54;
constexpr float kDashSpeed = 11.0f;
constexpr int kTotalAttackTicks = 72;
constexpr int kMinAttackTicks = 28;

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
               Asset<CharacterAssetData> asset,
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
      m_MovementState{},
      m_Health(health) {}

Result<CharacterAnimation> Player::GetAnimationToShow(AppCtx& ctx, const Arena& arena) const {
    switch (m_State) {
        case PlayerState::ATTACKING:
            return Ok(CharacterAnimation::Attacks);
        case PlayerState::DASHING:
            return Ok(CharacterAnimation::Dash);
        case PlayerState::IDLE:
        case PlayerState::RUNNING:
            break;
    }

    TRY(onGround, IsOnGround(ctx, arena));
    if (!onGround) {
        return Ok(m_MovementState.Velocity.y < 0.0f ? CharacterAnimation::Ascending : CharacterAnimation::Falling);
    }

    switch (m_State) {
        case PlayerState::RUNNING:
            return Ok(CharacterAnimation::Walk);
        case PlayerState::IDLE:
        case PlayerState::ATTACKING:
        case PlayerState::DASHING:
            return Ok(CharacterAnimation::Idle);
    }

    return Ok(CharacterAnimation::Idle);
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
        .x = m_MovementState.FacingRight ? m_Position.x - (width - anchorX) : m_Position.x - anchorX,
        .y = m_Position.y - anchorY,
        .w = width,
        .h = height,
    };
}

Result<std::optional<SDL_FRect>> Player::GetBaselineCollisionBox(AppCtx& ctx) const {
    TRY(asset, ctx.assets.GetAssetData(m_Asset));

    const auto sheet = asset.get().m_SpriteSheets.find(m_CurrentAnimation);
    if (sheet == asset.get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        return Ok(std::optional<SDL_FRect>{});
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame) % frames.size()];
    const SDL_FRect collisionBox = frame.m_CollisionBox;
    if (collisionBox.w <= 0.0f || collisionBox.h <= 0.0f) {
        return Ok(std::optional<SDL_FRect>{});
    }

    std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return Ok(std::optional<SDL_FRect>{});
    }

    const float collisionX =
        m_MovementState.FacingRight ? frame.m_Location.w - collisionBox.x - collisionBox.w : collisionBox.x;

    return Ok(std::optional<SDL_FRect>{SDL_FRect{
        .x = spriteRect->x + collisionX * kPlayerScale,
        .y = spriteRect->y + collisionBox.y * kPlayerScale,
        .w = collisionBox.w * kPlayerScale,
        .h = collisionBox.h * kPlayerScale,
    }});
}

Result<std::optional<WorldHitBox>> Player::GetCurrentHitBox(AppCtx& ctx) const {
    TRY(asset, ctx.assets.GetAssetData(m_Asset));

    const auto sheet = asset.get().m_SpriteSheets.find(m_CurrentAnimation);
    if (sheet == asset.get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        return Ok(std::nullopt);
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame) % frames.size()];
    const HitBox& hitBox = frame.m_HitBox;
    if (IsEmpty(hitBox)) {
        return Ok(std::nullopt);
    }

    std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return Ok(std::nullopt);
    }
    // need: facingRight, Position, Hitbox
    //const int worldX = spriteRect.value().x + hitBox.m_GridData.bounds.x * kPlayerScale;
    //const int worldY = spriteRect.value().y + hitBox.m_GridData.bounds.y * kPlayerScale;
    
    return Ok(WorldHitBox{hitBox, *spriteRect, m_MovementState.FacingRight});
}

Result<std::optional<WorldHurtBox>> Player::GetCurrentHurtBox(AppCtx& ctx) const {
    TRY(asset, ctx.assets.GetAssetData(m_Asset));

    const auto sheet = asset.get().m_SpriteSheets.find(m_CurrentAnimation);
    if (sheet == asset.get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        return Ok(std::nullopt);
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame) % frames.size()];
    const HurtBox& hurtBox = frame.m_HurtBox;
    if (IsEmpty(hurtBox)) {
        return Ok(std::nullopt);
    }

    std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return Ok(std::nullopt);
    }
    // need: facingRight, Position, Hitbox
    //const int worldX = spriteRect.value().x + hitBox.m_GridData.bounds.x * kPlayerScale;
    //const int worldY = spriteRect.value().y + hitBox.m_GridData.bounds.y * kPlayerScale;
    
    return Ok(WorldHurtBox{hurtBox, *spriteRect, m_MovementState.FacingRight});
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

Player::CurrentPlayerInput Player::GatherInput(AppCtx& ctx) {
    CurrentPlayerInput input{};
    input.JumpPressed = HasQueuedAction(m_InputQueue, InputAction::JUMP);
    input.DashPressed = HasQueuedAction(m_InputQueue, InputAction::DASH);
    input.AttackPressed = HasQueuedAction(m_InputQueue, InputAction::ATTACK);
    input.MoveLeftHeld = IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::MOVE_LEFT);
    input.MoveRightHeld = IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::MOVE_RIGHT);
    input.AttackHeld = IsActionHeld(ctx.input, m_InputTranslationHelper, InputAction::ATTACK);
    m_InputQueue.clear();
    return input;
}

void Player::TickCooldowns(const CurrentPlayerInput& input) {
    if (m_MovementState.DashCooldownTicksRemaining > 0) {
        --m_MovementState.DashCooldownTicksRemaining;
    }

    if (m_MovementState.AttackInfo.IsActive()) {
        --m_MovementState.AttackInfo.TicksRemaining;

        if (m_MovementState.AttackInfo.MinimumTicksRemaining > 0) {
            --m_MovementState.AttackInfo.MinimumTicksRemaining;
        }

        if (m_MovementState.AttackInfo.MinimumTicksRemaining == 0 && !input.AttackHeld) {
            m_MovementState.AttackInfo.TicksRemaining = 0;
        }
    }
}

Result<Player::GroundInfo> Player::QueryGroundInfo(AppCtx& ctx, const Arena& arena) const {
    TRY(onGround, IsOnGround(ctx, arena));
    return Ok(GroundInfo{.OnGround = onGround});
}

void Player::RefreshAirOptionsFromGround(const GroundInfo& groundInfo) {
    if (groundInfo.OnGround) {
        m_MovementState.AirDashAvailable = true;
        m_MovementState.DashJumpAvailable = false;
    }
}

void Player::RefreshPostMoveAirOptions(const GroundInfo& groundInfo,
                                       const HorizontalStepResult& horizontal) {
    if (groundInfo.OnGround) {
        m_MovementState.AirDashAvailable = true;
        m_MovementState.DashJumpAvailable = false;
    } else if (horizontal.DashedThisTick && m_MovementState.AirDashAvailable) {
        m_MovementState.DashJumpAvailable = true;
    }
}

Player::HorizontalStepResult Player::ApplyHorizontalIntent(const CurrentPlayerInput& input) {
    HorizontalStepResult result{};

    if (m_MovementState.DashTicksRemaining > 0) {
        m_Position.x += m_DashDirection * kDashSpeed;
        m_MovementState.Velocity.y = 0.0f;
        --m_MovementState.DashTicksRemaining;
        result.DashedThisTick = true;
        result.MovedHorizontally = true;
        return result;
    }

    if (input.AttackHeld) {
        return result;
    }

    if (input.MoveLeftHeld != input.MoveRightHeld) {
        const float direction = input.MoveLeftHeld ? -1.0f : 1.0f;
        m_Position.x += direction * kWalkSpeed;
        m_MovementState.FacingRight = direction > 0.0f;
        result.MovedHorizontally = true;
    }

    return result;
}

void Player::ApplyVerticalMotion(const HorizontalStepResult& horizontal) {
    if (horizontal.DashedThisTick) {
        return;
    }

    m_MovementState.Velocity.y += kGravity;
    m_Position.y += m_MovementState.Velocity.y;
}

Result<Player::CollisionResolution> Player::ResolveArenaCollisions(AppCtx& ctx,
                                                                   const Arena& arena) {
    CollisionResolution resolution{};

    TRY(arenaAsset, ctx.assets.GetAssetData(arena.asset));
    TRY(playerCollisionBox, GetBaselineCollisionBox(ctx));
    if (!playerCollisionBox) {
        return Ok(resolution);
    }

    const std::vector<SDL_FRect>& solidBoxes = arenaAsset.get().m_CollisionBoxes;
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

            resolution.Collided = true;
            if (push.y < 0.0f) {
                resolution.HitFloor = true;
            } else if (push.y > 0.0f) {
                resolution.HitCeiling = true;
            } else if (push.x > 0.0f) {
                resolution.HitLeftWall = true;
            } else if (push.x < 0.0f) {
                resolution.HitRightWall = true;
            }

            moved = true;
        }

        if (!moved) {
            break;
        }
    }

    return Ok(resolution);
}

void Player::ApplyCollisionResult(const CollisionResolution& resolution) {
    if (resolution.HitFloor && m_MovementState.Velocity.y > 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
    }

    if (resolution.HitCeiling && m_MovementState.Velocity.y < 0.0f) {
        m_MovementState.Velocity.y = 0.0f;
    }
}

void Player::UpdatePlayerState(const CurrentPlayerInput& input) {
    if (m_MovementState.DashTicksRemaining > 0) {
        m_State = PlayerState::DASHING;
    } else if (m_MovementState.AttackInfo.MinimumTicksRemaining > 0 || 
        (m_MovementState.AttackInfo.TicksRemaining && input.AttackHeld)) {
        m_State = PlayerState::ATTACKING;
    } else if (input.MoveLeftHeld != input.MoveRightHeld) {
        m_State = PlayerState::RUNNING;
    } else {
        m_State = PlayerState::IDLE;
    }
}


///////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////// 
/////////////////////////////////////////////////////////////////////////////////////////

void Player::RefreshAirOptions() {
    m_MovementState.AirDashAvailable = true;
    m_MovementState.DashJumpAvailable = false;
}

void Player::TryStartDash(const CurrentPlayerInput& input) {
    if (!input.DashPressed) {
        return;
    }

    if (m_MovementState.DashTicksRemaining != 0 || m_MovementState.DashCooldownTicksRemaining != 0) {
        return;
    }

    if (!m_MovementState.isGrounded && !m_MovementState.AirDashAvailable) {
        return;
    }

    if (input.MoveLeftHeld != input.MoveRightHeld) {
        m_MovementState.DashDirection = input.MoveLeftHeld ? -1.0f : 1.0f;
    } else {
        m_MovementState.DashDirection = m_MovementState.FacingRight ? 1.0f : -1.0f;
    }

    m_MovementState.FacingRight = m_MovementState.DashDirection > 0.0f;
    m_MovementState.DashTicksRemaining = kDashTicks;
    m_MovementState.DashCooldownTicksRemaining = kDashCooldownTicks;
    m_MovementState.Velocity.y = 0.0f;
    m_MovementState.Velocity.x = m_MovementState.DashDirection * kDashSpeed;

    if (!m_MovementState.isGrounded) {
        m_MovementState.AirDashAvailable = false;
        m_MovementState.DashJumpAvailable = true;
    }
}

void Player::TryStartAttack(const CurrentPlayerInput& input) {
    if (!m_MovementState.AttackInfo.IsActive() && input.AttackPressed) {
        m_MovementState.AttackInfo.TicksRemaining = kTotalAttackTicks;
        m_MovementState.AttackInfo.MinimumTicksRemaining = kMinAttackTicks;
        m_MovementState.Velocity.y = 0.0f;
        m_MovementState.Velocity.x = 0.0f;
    }
}

void Player::TryApplyJump(const CurrentPlayerInput& input) {
    if (!input.JumpPressed) {
        return;
    }
    if (m_MovementState.isGrounded) {
        m_MovementState.Velocity.y = kJumpVelocity;
    } else if (m_MovementState.DashJumpAvailable) {
        m_MovementState.Velocity.y = kJumpVelocity;
        m_MovementState.DashJumpAvailable = false;
    }
}

void Player::TryApplyMove(const CurrentPlayerInput& input) {
    if (input.MoveLeftHeld != input.MoveRightHeld) {
        const float direction = input.MoveLeftHeld ? -1.0f : 1.0f;
        m_MovementState.Velocity.x += direction * kWalkAcceleration * kAirMovementFactor;
        m_MovementState.FacingRight = m_MovementState.Velocity.x > 0.0f;
    }
}

void Player::ApplyGravity() {
    m_MovementState.Velocity.y += kGravity;
}

void Player::ApplyFriction() {
    m_MovementState.Velocity.y = std::min(m_MovementState.Velocity.y, kMaxFallSpeed);

    const float friction = m_MovementState.isGrounded ? kGroundFriction : kAirFriction;
    if (m_MovementState.Velocity.x > 0.0f) {
        m_MovementState.Velocity.x = std::clamp(m_MovementState.Velocity.x - friction, 0.0f, kWalkSpeed);
    } else if (m_MovementState.Velocity.x < 0.0f) {
        m_MovementState.Velocity.x = std::clamp(m_MovementState.Velocity.x + friction, -kWalkSpeed, 0.0f);
    }
}

void Player::ApplyVelocity() {
    m_Position.x += m_MovementState.Velocity.x;
    m_Position.y += m_MovementState.Velocity.y;
}

void Player::ApplyMoves(const CurrentPlayerInput& input) {
    TryStartDash(input);
    if (IsDashActive()) {
        --m_MovementState.DashTicksRemaining;
        return;
    }
    TryStartAttack(input);
    if (m_MovementState.AttackInfo.IsActive())
        return;

    TryApplyJump(input);
    TryApplyMove(input);
    ApplyGravity();
    ApplyFriction();
    return;
}

Result<void> Player::Movement(AppCtx& ctx, const Arena& arena) {
    const CurrentPlayerInput input = GatherInput(ctx);

    // isGrounded should already be known from the Collision detection
    TRY(groundState, QueryGroundInfo(ctx, arena));
    m_MovementState.isGrounded = groundState.OnGround;

    // Tick Down
    TickCooldowns(input);

    if(m_MovementState.isGrounded) {
        RefreshAirOptions();
    }

    ApplyMoves(input);

    // ApplyVelocityToPosition
    ApplyVelocity();
    // Dash
    
        // skip

    // Attack
    TRY(collision, ResolveArenaCollisions(ctx, arena));
    ApplyCollisionResult(collision);
    UpdatePlayerState(input);
    return Ok();
}

// Result<void> Player::TickGameLogic(AppCtx& ctx, const Arena& arena) {
//     const CurrentPlayerInput input = GatherInput(ctx);

//     TickCooldowns();

//     TRY(groundBefore, QueryGroundInfo(ctx, arena));
//     RefreshAirOptionsFromGround(groundBefore);

//     TryStartDash(input, groundBefore);

//     const HorizontalStepResult horizontal = ApplyHorizontalIntent(input);
//     TryApplyJump(input, groundBefore, horizontal);
//     ApplyVerticalMotion(horizontal);

//     TRY(collision, ResolveArenaCollisions(ctx, arena));
//     ApplyCollisionResult(collision);

//     TRY(groundAfter, QueryGroundInfo(ctx, arena));
//     RefreshPostMoveAirOptions(groundAfter, horizontal);

//     UpdatePlayerState(input, horizontal);
//     return Ok();
// }

Vec2 Player::LocalFramePointToBaselinePoint(const CharacterSpriteSheetFrame& frame,
                                            const SDL_FRect& spriteRect,
                                            Vec2 localPoint,
                                            bool facingRight) {
    const float localX = facingRight ? frame.m_Location.w - localPoint.x : localPoint.x;

    return Vec2{
        spriteRect.x + localX * kPlayerScale,
        spriteRect.y + localPoint.y * kPlayerScale,
    };
}

Vec2 Player::MapBaselinePointToArenaPoint(Vec2 point, const SDL_Rect& arenaDimensions) {
    const SDL_FRect mapped = MapBaselineRectToArena(
        SDL_FRect{
            .x = point.x,
            .y = point.y,
            .w = 1.0f,
            .h = 1.0f,
        },
        arenaDimensions);

    return Vec2{mapped.x, mapped.y};
}

Result<void> Player::DispatchSwordFrameEffects(AppCtx& ctx,
                                               const Arena& arena,
                                               const CharacterSpriteSheetFrame& frame,
                                               std::span<const FrameEffectMask> swordMasks) {
    const bool frameChanged = m_CurrentAnimation != m_PreviousEffectAnimation ||
                              m_CurrentAnimationFrame != m_PreviousEffectFrameIndex;
    m_PreviousEffectAnimation = m_CurrentAnimation;
    m_PreviousEffectFrameIndex = m_CurrentAnimationFrame;

    if (!frameChanged) {
        return Ok();
    }

    if (m_CurrentAnimation != CharacterAnimation::Attacks) {
        return Ok();
    }

    const std::size_t frameIndex = static_cast<std::size_t>(m_CurrentAnimationFrame);
    if (frameIndex >= swordMasks.size()) {
        return Ok();
    }

    const FrameEffectMask& mask = swordMasks[frameIndex];
    if (mask.Points.empty()) {
        return Ok();
    }

    const std::optional<SDL_FRect> spriteRect = GetBaselineSpriteRect(frame);
    if (!spriteRect) {
        return Ok();
    }

    constexpr int particlesPerAttackFrame = 6;
    std::uniform_int_distribution<std::size_t> pointDistribution(0, mask.Points.size() - 1);

    for (int i = 0; i < particlesPerAttackFrame; ++i) {
        const Vec2 localPoint = mask.Points[pointDistribution(m_EffectRandom)];

        const Vec2 baselinePoint =
            LocalFramePointToBaselinePoint(frame, *spriteRect, localPoint, m_MovementState.FacingRight);
        const Vec2 arenaPoint = MapBaselinePointToArenaPoint(baselinePoint, arena.dimensions);

        ctx.eventDispatcher.Enqueue(PlayerParticleEffectEvent{
            .Type = PlayerParticleEffectType::SwordFire,
            .Position = arenaPoint,
            .Velocity =
                Vec2{
                    m_MovementState.FacingRight ? 90.0f : -90.0f,
                    m_MovementState.Velocity.y - 120.0f,
                },
            .FacingRight = m_MovementState.FacingRight,
            .Strength = 1.0f,
        });
    }

    return Ok();
}

Result<void> Player::TickAnimations(AppCtx& ctx, const Arena& arena) {
    TRY(asset, ctx.assets.GetAssetData(m_Asset));

    TRY(animation, GetAnimationToShow(ctx, arena));
    if (animation != m_CurrentAnimation) {
        m_CurrentAnimationFrame = 0;
    }
    m_CurrentAnimation = animation;
    const auto sheet = asset.get().m_SpriteSheets.find(animation);
    if (sheet == asset.get().m_SpriteSheets.end() || sheet->second.m_Frames.empty()) {
        m_CurrentAnimationFrame = 0;
        return Ok();
    }

    const std::vector<CharacterSpriteSheetFrame>& frames = sheet->second.m_Frames;
    const bool animationChanged = animation != m_CurrentAnimation;

    if (animationChanged) {
        m_CurrentAnimationFrame = 0;
    } else {
        m_CurrentAnimationFrame = (m_CurrentAnimationFrame + 1) % static_cast<int>(frames.size());
    }

    const CharacterSpriteSheetFrame& frame =
        frames[static_cast<std::size_t>(m_CurrentAnimationFrame)];

    // maybe not dispatch every frame, maybe do the player events etc also with our event system?
    const auto swordFireMasks =
        sheet->second.GetEffectMasks(CharacterFrameEffectMaskType::SwordFire);
    TRY_VOID(DispatchSwordFrameEffects(ctx, arena, frame, swordFireMasks));

    return Ok();
}

Result<bool> Player::IsOnGround(AppCtx& ctx, const Arena& arena) const {
    TRY(arenaAsset, ctx.assets.GetAssetData(arena.asset));

    TRY(playerCollisionBox, GetBaselineCollisionBox(ctx));
    if (!playerCollisionBox) {
        return Ok(false);
    }

    playerCollisionBox->y += kGroundProbeDistance;

    for (const SDL_FRect& solidBox : arenaAsset.get().m_CollisionBoxes) {
        if (SDL_HasRectIntersectionFloat(&*playerCollisionBox, &solidBox)) {
            return Ok(true);
        }
    }

    return Ok(false);
}

Result<void> Player::Render(AppCtx& ctx, const Arena& arena) const {
    TRY(asset, ctx.assets.GetAssetData(m_Asset));

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
    params.flip = m_MovementState.FacingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    return ctx.renderer.DrawTexture(sheet->second.m_Texture.get(), params);
}

Result<void> Player::RenderCollisionBox(AppCtx& ctx, const Arena& arena) const {
    TRY(collisionBox, GetBaselineCollisionBox(ctx));
    if (!collisionBox) {
        return Ok();
    }

    return ctx.renderer.DrawRect(MapBaselineRectToArena(*collisionBox, arena.dimensions),
                                 Color{255, 230, 0, 255});
}

namespace {

SDL_FRect TransformRectToWorldspace(const SDL_FRect& localRect,
                                    const SDL_FRect& spriteRect,
                                    bool facingRight) {
    constexpr float kPlayerScale = 0.4f;

    const float scaledX = localRect.x * kPlayerScale;
    const float scaledY = localRect.y * kPlayerScale;
    const float scaledW = localRect.w * kPlayerScale;
    const float scaledH = localRect.h * kPlayerScale;

    return SDL_FRect{
        .x = facingRight
                 ? (spriteRect.x + spriteRect.w - scaledX - scaledW)
                 : (spriteRect.x + scaledX),
        .y = spriteRect.y + scaledY,
        .w = scaledW,
        .h = scaledH,
    };
}

}  // namespace

Result<void> Player::RenderHitBoxes(AppCtx& ctx, const Arena& arena) const {
    TRY(worldHitBox, GetCurrentHitBox(ctx));
    if (!worldHitBox) {
        return Ok();
    }

    const SDL_FRect worldRect =
        TransformRectToWorldspace(worldHitBox->hitBox.get().m_GridData.bounds,
                                  worldHitBox->spriteRect,
                                  worldHitBox->facingRight);

    return ctx.renderer.DrawRect(MapBaselineRectToArena(worldRect, arena.dimensions),
                                 Color{255, 0, 0, 255});
}

Result<void> Player::RenderHurtBoxes(AppCtx& ctx, const Arena& arena) const {
    TRY(worldHurtBox, GetCurrentHurtBox(ctx));
    if (!worldHurtBox) {
        return Ok();
    }

    for (const auto& [value, subHurtBox] : worldHurtBox->hurtBox.get().m_SubHurtBoxes) {
        (void)value;

        const SDL_FRect worldRect =
            TransformRectToWorldspace(subHurtBox.m_GridData.bounds,
                                      worldHurtBox->spriteRect,
                                      worldHurtBox->facingRight);

        TRY_VOID(ctx.renderer.DrawRect(MapBaselineRectToArena(worldRect, arena.dimensions),
                                       Color{0, 0, 255, 255}));
    }

    return Ok();
}

}  // namespace sop
