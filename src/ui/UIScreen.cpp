#include "smashorpass/ui/UIScreen.hpp"

#include "smashorpass/core/AppCtx.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "smashorpass/ui/UIBuilder.hpp"
#include "spdlog/spdlog.h"

namespace sop {

UIScreen::UIScreen(AppCtx&) {}

EventFlow UIScreen::OnEvent(AppCtx& ctx, const Event& event) {
    bool consumed = false;

    EventDispatcher::Dispatch<MouseButtonEvent>(event,
                                                [this, &ctx, &consumed](const MouseButtonEvent& e) {
                                                    if (!e.Down)
                                                        return;
                                                    Vec2 mousePos{e.X, e.Y};

                                                    for (UIWidget& w : m_Widgets) {
                                                        if (w.Kind != WidgetKind::Button)
                                                            continue;

                                                        if (!PointInRect(mousePos, w.LayoutRect))
                                                            continue;

                                                        auto& d = std::get<ButtonData>(w.Data);
                                                        if (d.OnClick)
                                                            d.OnClick(ctx, d);
                                                        consumed = true;
                                                        return;
                                                    }
                                                });

    EventDispatcher::Dispatch<MouseMovedEvent>(
        event, [this, &ctx, &consumed](const MouseMovedEvent& e) {
            Vec2 mousePos{e.X, e.Y};

            for (UIWidget& w : m_Widgets) {
                if (w.Kind != WidgetKind::Button)
                    continue;

                const bool hover = PointInRect(mousePos, w.LayoutRect);
                auto& d = std::get<ButtonData>(w.Data);
                if (hover) {
                    if (d.OnHover)
                        d.OnHover(ctx, d);
                    consumed = true;
                }
            }
        });

    return consumed ? EventFlow::Consumed : EventFlow::Passed;
}

void UIScreen::OnUpdate(AppCtx&) {}

Result<void> UIScreen::OnRender(AppCtx& ctx) {
    Renderer& renderer = ctx.renderer;

    if (m_RebuildRequested) {
        m_Widgets.clear();
        m_Root = g_InvalidWidgetId;

        UIBuilder builder(*this);
        Build(builder);

        m_RebuildRequested = false;
    }

    if (m_Root == g_InvalidWidgetId)
        return Ok();

    TRY(outputSize, renderer.GetLogicalOutputSize());

    TRY(rootSize, MeasureWidget(ctx, m_Root));
    (void)rootSize;
    LayoutWidget(m_Root, SDL_FRect{0.0f, 0.0f, outputSize.x, outputSize.y});

    for (const UIWidget& w : m_Widgets)
        TRY_VOID(RenderWidget(ctx, w));

    return Ok();
}

void UIScreen::RebuildUI() {
    m_RebuildRequested = true;
}

Result<void> UIScreen::RenderWidget(AppCtx& ctx, const UIWidget& widget) {
    Renderer& renderer = ctx.renderer;

    switch (widget.Kind) {
        case WidgetKind::Stack:
        case WidgetKind::Column:
        case WidgetKind::Row:
        case WidgetKind::Align:
        case WidgetKind::Image:
            break;
        case WidgetKind::Label: {
            const auto& d = std::get<LabelData>(widget.Data);
            TRY_VOID(renderer.DrawText(
                d.Font, widget.LayoutRect.x, widget.LayoutRect.y, d.Text, d.TextColor));
            break;
        }
        case WidgetKind::Button: {
            const auto& d = std::get<ButtonData>(widget.Data);
            const SDL_FRect buttonRect = widget.LayoutRect;

            TRY_VOID(renderer.FillRect(buttonRect, d.BackgroundColor));
            TRY_VOID(renderer.DrawRect(buttonRect, d.BorderColor));

            TRY(textSize, renderer.MeasureText(d.Font, d.Text));

            const float textX = buttonRect.x + (buttonRect.w - textSize.x) * 0.5f;
            const float textY = buttonRect.y + (buttonRect.h - textSize.y) * 0.5f;

            TRY_VOID(renderer.DrawText(d.Font, textX, textY, d.Text, d.TextColor));

            break;
        }
    }

    return Ok();
}

Result<Vec2> UIScreen::MeasureWidget(AppCtx& ctx, UIWidgetId id) {
    Renderer& renderer = ctx.renderer;
    UIWidget& w = GetWidgetById(id);

    switch (w.Kind) {
        case WidgetKind::Label: {
            auto& d = std::get<LabelData>(w.Data);
            TRY(s, renderer.MeasureText(d.Font, d.Text));
            w.Measured = SDL_FRect{0, 0, s.x, s.y};
            return Ok(s);
        }
        case WidgetKind::Button: {
            auto& d = std::get<ButtonData>(w.Data);

            TRY(textSize, renderer.MeasureText(d.Font, d.Text));

            w.Measured = SDL_FRect{0.0f,
                                   0.0f,
                                   textSize.x + Theme::BUTTON_PADDING_X * 2.0f,
                                   textSize.y + Theme::BUTTON_PADDING_Y * 2.0f};

            return Ok(Vec2{w.Measured.w, w.Measured.h});
        }
        case WidgetKind::Column: {
            auto& d = std::get<ColumnData>(w.Data);
            float maxW = 0.0f;
            float totalH = 0.0f;
            int count = 0;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                TRY(cs, MeasureWidget(ctx, c));
                maxW = std::max(maxW, cs.x);
                totalH += cs.y;
                ++count;
            }

            if (count > 1)
                totalH += d.Spacing * static_cast<float>(count - 1);

            w.Measured = SDL_FRect{0, 0, maxW, totalH};
            return Ok(Vec2{maxW, totalH});
        }
        case WidgetKind::Row: {
            auto& d = std::get<RowData>(w.Data);
            float totalW = 0.0f;
            float maxH = 0.0f;
            int count = 0;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                TRY(cs, MeasureWidget(ctx, c));
                totalW += cs.x;
                maxH = std::max(maxH, cs.y);
                ++count;
            }

            if (count > 1)
                totalW += d.Spacing * static_cast<float>(count - 1);

            w.Measured = SDL_FRect{0, 0, totalW, maxH};
            return Ok(Vec2{totalW, maxH});
        }
        case WidgetKind::Stack: {
            float maxW = 0.0f;
            float maxH = 0.0f;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                TRY(cs, MeasureWidget(ctx, c));
                maxW = std::max(maxW, cs.x);
                maxH = std::max(maxH, cs.y);
            }

            w.Measured = SDL_FRect{0, 0, maxW, maxH};
            return Ok(Vec2{maxW, maxH});
        }
        case WidgetKind::Align: {
            if (w.FirstChild == g_InvalidWidgetId) {
                w.Measured = SDL_FRect{0, 0, 0, 0};
                return Ok(Vec2{});
            }

            TRY(childSize, MeasureWidget(ctx, w.FirstChild));
            w.Measured = SDL_FRect{0, 0, childSize.x, childSize.y};
            return Ok(childSize);
        }
        case WidgetKind::Image:
            w.Measured = SDL_FRect{0, 0, 0, 0};
            return Ok(Vec2{});
    }

    return Ok(Vec2{});
}

void UIScreen::LayoutWidget(UIWidgetId id, SDL_FRect rect) {
    UIWidget& w = GetWidgetById(id);
    w.LayoutRect = rect;

    switch (w.Kind) {
        case WidgetKind::Label:
        case WidgetKind::Button:
        case WidgetKind::Image:
            return;
        case WidgetKind::Column: {
            auto& d = std::get<ColumnData>(w.Data);
            float y = rect.y;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                UIWidget& child = GetWidgetById(c);

                float childX = rect.x;

                switch (GetHorizontalAlign(child.SelfAlignment)) {
                    case HorizontalAlign::Left:
                        childX = rect.x;
                        break;
                    case HorizontalAlign::Center:
                        childX = rect.x + (rect.w - child.Measured.w) * 0.5f;
                        break;
                    case HorizontalAlign::Right:
                        childX = rect.x + rect.w - child.Measured.w;
                        break;
                }

                SDL_FRect childRect{childX, y, child.Measured.w, child.Measured.h};

                LayoutWidget(c, childRect);
                y += child.Measured.h + d.Spacing;
            }
            break;
        }
        case WidgetKind::Row: {
            auto& d = std::get<RowData>(w.Data);
            float x = rect.x;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                UIWidget& child = GetWidgetById(c);

                float childY = rect.y;

                switch (GetVerticalAlign(child.SelfAlignment)) {
                    case VerticalAlign::Top:
                        childY = rect.y;
                        break;
                    case VerticalAlign::Center:
                        childY = rect.y + (rect.h - child.Measured.h) * 0.5f;
                        break;
                    case VerticalAlign::Bottom:
                        childY = rect.y + rect.h - child.Measured.h;
                        break;
                }

                SDL_FRect childRect{x, childY, child.Measured.w, child.Measured.h};

                LayoutWidget(c, childRect);
                x += child.Measured.w + d.Spacing;
            }
            break;
        }
        case WidgetKind::Stack: {
            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                LayoutWidget(c, rect);
            }
            return;
        }
        case WidgetKind::Align: {
            auto& d = std::get<AlignData>(w.Data);

            if (w.FirstChild == g_InvalidWidgetId)
                return;

            UIWidget& child = GetWidgetById(w.FirstChild);

            float childX = rect.x;
            float childY = rect.y;

            switch (GetHorizontalAlign(d.Anchor)) {
                case HorizontalAlign::Left:
                    childX = rect.x;
                    break;
                case HorizontalAlign::Center:
                    childX = rect.x + (rect.w - child.Measured.w) * 0.5f;
                    break;
                case HorizontalAlign::Right:
                    childX = rect.x + rect.w - child.Measured.w;
                    break;
            }

            switch (GetVerticalAlign(d.Anchor)) {
                case VerticalAlign::Top:
                    childY = rect.y;
                    break;
                case VerticalAlign::Center:
                    childY = rect.y + (rect.h - child.Measured.h) * 0.5f;
                    break;
                case VerticalAlign::Bottom:
                    childY = rect.y + rect.h - child.Measured.h;
                    break;
            }

            LayoutWidget(w.FirstChild,
                         SDL_FRect{childX, childY, child.Measured.w, child.Measured.h});
            return;
        }
    }
}

bool UIScreen::PointInRect(const Vec2& point, const SDL_FRect& rect) const {
    return point.x >= rect.x && point.x <= rect.x + rect.w && point.y >= rect.y &&
           point.y <= rect.y + rect.h;
}

void UIScreen::AddChild(UIWidgetId parent, UIWidgetId child) {
    SOP_ASSERT(parent < m_Widgets.size(), "Invalid parent widget ID");
    SOP_ASSERT(child < m_Widgets.size(), "Invalid child widget ID");
    SOP_ASSERT(parent != child, "Parent and child cannot be the same");

    UIWidget& p = m_Widgets[parent];
    UIWidget& c = m_Widgets[child];

    SOP_ASSERT(c.Parent == g_InvalidWidgetId, "Widget already has a parent");

    c.Parent = parent;

    if (p.FirstChild == g_InvalidWidgetId) {
        p.FirstChild = child;
        p.LastChild = child;
        return;
    }

    m_Widgets[p.LastChild].NextSibling = child;
    p.LastChild = child;
}

void UIScreen::SetRoot(UIWidgetId id) {
    SOP_ASSERT(id < m_Widgets.size(), "Invalid root widget ID");
    m_Root = id;
}
}  // namespace sop
