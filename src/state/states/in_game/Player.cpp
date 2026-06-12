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
constexpr float kGroundProbeDistance = 5.0f;

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
      m_State(PlayerActionState::IDLE),
      m_MovementConfig{},
      m_MovementState{},
      m_Health(health) {
    m_MovementState.FacingRight = facingRight;
}

Result<CharacterAnimation> Player::GetAnimationToShow(AppCtx& ctx, const Arena& arena) const {
    switch (m_State) {
        case PlayerActionState::ATTACKING:
            return Ok(CharacterAnimation::Attacks);
        case PlayerActionState::DASHING:
            return Ok(CharacterAnimation::Dash);
        case PlayerActionState::IDLE:
        case PlayerActionState::RUNNING:
            break;
    }

    TRY(onGround, IsOnGround(ctx, arena));
    if (!onGround) {
        return Ok(m_MovementState.Velocity.y < 0.0f ? CharacterAnimation::Ascending : CharacterAnimation::Falling);
    }

    switch (m_State) {
        case PlayerActionState::RUNNING:
            return Ok(CharacterAnimation::Walk);
        case PlayerActionState::IDLE:
        case PlayerActionState::ATTACKING:
        case PlayerActionState::DASHING:
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
        .x = m_FacingRight ? m_Position.x - (width - anchorX) : m_Position.x - anchorX,
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
        m_FacingRight ? frame.m_Location.w - collisionBox.x - collisionBox.w : collisionBox.x;

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
    
    return Ok(WorldHitBox{hitBox, *spriteRect, m_FacingRight});
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
    
    return Ok(WorldHurtBox{hurtBox, *spriteRect, m_FacingRight});
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
            LocalFramePointToBaselinePoint(frame, *spriteRect, localPoint, m_FacingRight);
        const Vec2 arenaPoint = MapBaselinePointToArenaPoint(baselinePoint, arena.dimensions);

        ctx.eventDispatcher.Enqueue(PlayerParticleEffectEvent{
            .Type = PlayerParticleEffectType::SwordFire,
            .Position = arenaPoint,
            .Velocity =
                Vec2{
                    m_FacingRight ? 90.0f : -90.0f,
                    m_MovementState.Velocity.y - 120.0f,
                },
            .FacingRight = m_FacingRight,
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
    params.flip = m_FacingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

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
