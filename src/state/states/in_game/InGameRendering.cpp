#include <array>
#include <cstddef>

#include "smashorpass/core/Color.hpp"
#include "smashorpass/state/states/in_game/InGameState.hpp"
#include "smashorpass/util.hpp"

namespace sop {

namespace {

constexpr float kMarkerHalfWidth = 12.0f;
constexpr float kMarkerHeight = 16.0f;
constexpr float kMarkerGap = 10.0f;
constexpr std::array<Color, 4> kMarkerColors{
    Color{255, 40, 40, 255},
    Color{40, 110, 255, 255},
    Color{45, 210, 95, 255},
    Color{255, 220, 45, 255},
};

[[nodiscard]] SDL_FColor ToVertexColor(const Color color) {
    return SDL_FColor{
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f,
    };
}

[[nodiscard]] std::array<SDL_Vertex, 3> MakeMarkerTriangle(const float tipX,
                                                           const float tipY,
                                                           const SDL_FColor color) {
    // Renders simple colored Triangles over players so you can distinguish them
    const float baseY = tipY - kMarkerHeight;

    return std::array<SDL_Vertex, 3>{
        SDL_Vertex{
            .position = SDL_FPoint{.x = tipX - kMarkerHalfWidth, .y = baseY},
            .color = color,
            .tex_coord = SDL_FPoint{.x = 0.0f, .y = 0.0f},
        },
        SDL_Vertex{
            .position = SDL_FPoint{.x = tipX + kMarkerHalfWidth, .y = baseY},
            .color = color,
            .tex_coord = SDL_FPoint{.x = 0.0f, .y = 0.0f},
        },
        SDL_Vertex{
            .position = SDL_FPoint{.x = tipX, .y = tipY},
            .color = color,
            .tex_coord = SDL_FPoint{.x = 0.0f, .y = 0.0f},
        },
    };
}

}  // namespace

Result<void> InGameState::RenderDebugBoxes(AppCtx& ctx) {
    // Render Boxes;
    // Collisionbox, Hit & Hurt or Combat Hit & Hurt
    // Combat Hit are only the cells that are checked for overlap
    // Combat Hurt are the Subhurtboxes that are used for the check
    if (!ctx.debugRender.renderPlayerBoxes) {
        return Ok();
    }

    for (std::size_t i = 0; i < m_Players.size(); ++i) {
        const PlayerDebugRenderOptions& debugOptions = m_PlayerDebugRenderOptions[i];
        const PlayerCombatDebugData& debugData = m_PlayerCombatDebugData[i];

        if (!debugOptions.enabled) {
            continue;
        }

        if (debugOptions.collisionBox) {
            TRY_VOID(m_Players[i].RenderCollisionBox(ctx, m_Arena));
        }

        if (debugOptions.hitBoxes) {
            TRY_VOID(m_Players[i].RenderHitBoxes(ctx, m_Arena));
        }

        if (debugOptions.hurtBoxes) {
            TRY_VOID(m_Players[i].RenderHurtBoxes(ctx, m_Arena));
        }

        if (debugOptions.combatHitBoxes) {
            for (const SDL_FRect& rect : debugData.hitBoxBounds) {
                TRY_VOID(ctx.renderer.DrawRect(MapBaselineRectToArena(rect, m_Arena.dimensions),
                                               Color{255, 0, 0, 255}));
            }
        }

        if (debugOptions.combatHurtBoxes) {
            for (const SDL_FRect& rect : debugData.hurtBoxBounds) {
                TRY_VOID(ctx.renderer.DrawRect(MapBaselineRectToArena(rect, m_Arena.dimensions),
                                               Color{0, 0, 255, 255}));
            }
        }
    }

    return Ok();
}

Result<void> InGameState::RenderBackdrop(AppCtx& ctx) {
    TRY(arenaAsset, ctx.assets.GetAssetData(m_Arena.asset));

    SDL_FRect rect{};
    SDL_RectToFRect(&m_Arena.dimensions, &rect);
    return ctx.renderer.DrawTexture(arenaAsset.get().m_Background.get(), rect);
}

Result<void> InGameState::RenderPlayers(AppCtx& ctx) {
    for (const Player& player : m_Players) {
        TRY_VOID(player.Render(ctx, m_Arena));
    }
    return Ok();
}

Result<void> InGameState::RenderEffects(AppCtx& ctx) {
    return ctx.particleSystem.Render(ctx);
}

Result<void> InGameState::RenderForeground(AppCtx& ctx) {
    TRY(arenaAsset, ctx.assets.GetAssetData(m_Arena.asset));

    SDL_FRect rect{};
    SDL_RectToFRect(&m_Arena.dimensions, &rect);
    return ctx.renderer.DrawTexture(arenaAsset.get().m_Foreground.get(), rect);
}

Result<void> InGameState::RenderPlayerMarkers(AppCtx& ctx) {
    for (std::size_t playerIndex = 0; playerIndex < m_Players.size(); ++playerIndex) {
        TRY(markerAnchor, m_Players[playerIndex].GetBaselineMarkerAnchor(ctx));
        if (!markerAnchor) {
            continue;
        }

        const SDL_FRect mappedAnchor = MapBaselineRectToArena(
            SDL_FRect{
                .x = markerAnchor->x,
                .y = markerAnchor->y,
                .w = 0.0f,
                .h = 0.0f,
            },
            m_Arena.dimensions);

        const float tipX = mappedAnchor.x;
        const float tipY = mappedAnchor.y - kMarkerGap;
        const Color markerColor = kMarkerColors[playerIndex % kMarkerColors.size()];
        const SDL_FColor vertexColor = ToVertexColor(markerColor);
        const std::array<SDL_Vertex, 3> vertices = MakeMarkerTriangle(tipX, tipY, vertexColor);

        TRY_VOID(ctx.renderer.DrawGeometry(nullptr, vertices));
    }

    return Ok();
}

Result<void> InGameState::RenderArenaCollisionBoxes(AppCtx& ctx) {
    if (!ctx.debugRender.renderArenaCollisionBoxes) {
        return Ok();
    }

    TRY(arenaAsset, ctx.assets.GetAssetData(m_Arena.asset));

    for (const SDL_FRect& collisionBox : arenaAsset.get().m_CollisionBoxes) {
        TRY_VOID(ctx.renderer.DrawRect(MapBaselineRectToArena(collisionBox, m_Arena.dimensions),
                                       Color{0, 255, 0, 255}));
    }

    return Ok();
}

Result<void> InGameState::RenderUi(AppCtx& ctx) {
    TRY_VOID(m_GameScreen.OnRender(ctx));

    if (m_Paused) {
        TRY_VOID(ActivePauseScreen().OnRender(ctx));
    }

    return Ok();
}

Result<void> InGameState::RenderLoadingScreen(AppCtx& ctx) {
    const SDL_FPoint logicalSize = ctx.displayMetrics.LogicalSize();
    const SDL_FRect screenRect{
        .x = 0.0f,
        .y = 0.0f,
        .w = logicalSize.x,
        .h = logicalSize.y,
    };

    TRY_VOID(ctx.renderer.FillRect(screenRect, Color{18, 18, 24, 255}));

    const SDL_FRect panelRect{
        .x = logicalSize.x * 0.5f - 180.0f,
        .y = logicalSize.y * 0.5f - 70.0f,
        .w = 360.0f,
        .h = 140.0f,
    };
    TRY_VOID(ctx.renderer.FillRect(panelRect, Color{32, 34, 42, 245}));
    TRY_VOID(ctx.renderer.DrawRect(panelRect, Color{110, 118, 140, 255}));

    constexpr char kLoadingText[] = "Loading match";
    TRY(textSize, ctx.renderer.MeasureText(FontId::Medium, kLoadingText));
    TRY_VOID(ctx.renderer.DrawText(FontId::Medium,
                                   panelRect.x + (panelRect.w - textSize.x) * 0.5f,
                                   panelRect.y + 28.0f,
                                   kLoadingText,
                                   Color{235, 238, 245, 255}));

    constexpr float barWidth = 52.0f;
    constexpr float barHeight = 12.0f;
    constexpr float barGap = 12.0f;
    const float barsWidth = barWidth * 3.0f + barGap * 2.0f;
    const float barStartX = panelRect.x + (panelRect.w - barsWidth) * 0.5f;
    const float barY = panelRect.y + 82.0f;
    constexpr std::array<Color, 3> barColors{
        Color{255, 82, 82, 255},
        Color{80, 160, 255, 255},
        Color{255, 220, 75, 255},
    };

    for (std::size_t i = 0; i < barColors.size(); ++i) {
        const SDL_FRect barRect{
            .x = barStartX + static_cast<float>(i) * (barWidth + barGap),
            .y = barY,
            .w = barWidth,
            .h = barHeight,
        };
        TRY_VOID(ctx.renderer.FillRect(barRect, barColors[i]));
    }

    return Ok();
}

}  // namespace sop
