#include "smashorpass/ui/UIScreen.hpp"

#include "smashorpass/core/Base.hpp"
#include "smashorpass/rendering/Renderer.hpp"
#include "spdlog/spdlog.h"

#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

UIScreen::UIScreen(ApplicationState stateToRepresent, EventDispatcher& dispatcher)
    : m_ApplicationStateToRepresent(stateToRepresent), m_EventDispatcher(dispatcher) {}

void UIScreen::OnEvent(const Event& event) {
    EventDispatcher::Dispatch<MouseButtonEvent>(event, [this](const MouseButtonEvent& e) {
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
                d.OnClick(m_EventDispatcher, d);
        }
    });

    EventDispatcher::Dispatch<MouseMovedEvent>(event, [this](const MouseMovedEvent& e) {
        Vec2 mousePos{e.X, e.Y};

        for (UIWidget& w : m_Widgets) {
            if (w.Kind != WidgetKind::Button)
                continue;

            const bool hover = PointInRect(mousePos, w.LayoutRect);
            auto& d = std::get<ButtonData>(w.Data);
            if (hover && d.OnHover)
                d.OnHover(m_EventDispatcher, d);
        }
    });
}

void UIScreen::OnUpdate() {}

void UIScreen::OnRender(Renderer& renderer) {
    if (m_RebuildRequested) {
        m_Widgets.clear();
        m_Root = g_InvalidWidgetId;

        UIBuilder builder(*this);
        Build(builder);

        m_RebuildRequested = false;
    }

    if (m_Root == g_InvalidWidgetId)
        return;

    const SDL_FPoint outputSize = renderer.GetLogicalOutputSize();

    MeasureWidget(m_Root, renderer);
    LayoutWidget(m_Root, SDL_FRect{0.0f, 0.0f, outputSize.x, outputSize.y});

    for (const UIWidget& w : m_Widgets)
        RenderWidget(renderer, w);
}

void UIScreen::RebuildUI() {
    m_RebuildRequested = true;
}

void UIScreen::RenderWidget(Renderer& renderer, const UIWidget& widget) {
    switch (widget.Kind) {
        case WidgetKind::Stack:
        case WidgetKind::Column:
        case WidgetKind::Row:
        case WidgetKind::Align:
        case WidgetKind::Image:
            break;
        case WidgetKind::Label: {
            const auto& d = std::get<LabelData>(widget.Data);
            renderer.DrawText(d.Font, 
                widget.LayoutRect.x, widget.LayoutRect.y, d.Text, d.TextColor);
            break;
        }
        case WidgetKind::Button: {
            const auto& d = std::get<ButtonData>(widget.Data);
            const SDL_FRect buttonRect = widget.LayoutRect;

            renderer.FillRect(buttonRect, d.BackgroundColor);
            renderer.DrawRect(buttonRect, d.BorderColor);

            Vec2 textSize = renderer.MeasureText(d.Font, d.Text);

            const float textX = buttonRect.x + (buttonRect.w - textSize.x) * 0.5f;
            const float textY = buttonRect.y + (buttonRect.h - textSize.y) * 0.5f;

            renderer.DrawText(d.Font, textX, textY, d.Text, d.TextColor);

            break;
        }
    }
}

Vec2 UIScreen::MeasureWidget(UIWidgetId id, Renderer& renderer) {
    
    UIWidget& w = GetWidgetById(id);

    switch (w.Kind) {
        case WidgetKind::Label: {
            auto& d = std::get<LabelData>(w.Data);
            Vec2 s = renderer.MeasureText(d.Font, d.Text);
            w.Measured = SDL_FRect{0, 0, s.x, s.y};
            return s;
        }
        case WidgetKind::Button: {
            auto& d = std::get<ButtonData>(w.Data);

            Vec2 textSize = renderer.MeasureText(d.Font, d.Text);

            w.Measured = SDL_FRect{0.0f,
                                   0.0f,
                                   textSize.x + Theme::BUTTON_PADDING_X * 2.0f,
                                   textSize.y + Theme::BUTTON_PADDING_Y * 2.0f};

            return Vec2{w.Measured.w, w.Measured.h};
        }
        case WidgetKind::Column: {
            auto& d = std::get<ColumnData>(w.Data);
            float maxW = 0.0f;
            float totalH = 0.0f;
            int count = 0;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                Vec2 cs = MeasureWidget(c, renderer);
                maxW = std::max(maxW, cs.x);
                totalH += cs.y;
                ++count;
            }

            if (count > 1)
                totalH += d.Spacing * static_cast<float>(count - 1);

            w.Measured = SDL_FRect{0, 0, maxW, totalH};
            return Vec2{maxW, totalH};
        }
        case WidgetKind::Row: {
            auto& d = std::get<RowData>(w.Data);
            float totalW = 0.0f;
            float maxH = 0.0f;
            int count = 0;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                Vec2 cs = MeasureWidget(c, renderer);
                totalW += cs.x;
                maxH = std::max(maxH, cs.y);
                ++count;
            }

            if (count > 1)
                totalW += d.Spacing * static_cast<float>(count - 1);

            w.Measured = SDL_FRect{0, 0, totalW, maxH};
            return Vec2{totalW, maxH};
        }
        case WidgetKind::Stack: {
            float maxW = 0.0f;
            float maxH = 0.0f;

            for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId;
                 c = GetWidgetById(c).NextSibling) {
                Vec2 cs = MeasureWidget(c, renderer);
                maxW = std::max(maxW, cs.x);
                maxH = std::max(maxH, cs.y);
            }

            w.Measured = SDL_FRect{0, 0, maxW, maxH};
            return Vec2{maxW, maxH};
        }
        case WidgetKind::Align: {
            if (w.FirstChild == g_InvalidWidgetId) {
                w.Measured = SDL_FRect{0, 0, 0, 0};
                return Vec2{};
            }

            Vec2 childSize = MeasureWidget(w.FirstChild, renderer);
            w.Measured = SDL_FRect{0, 0, childSize.x, childSize.y};
            return childSize;
        }
        case WidgetKind::Image:
            w.Measured = SDL_FRect{0, 0, 0, 0};
            return Vec2{};
    }

    return Vec2{};
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
