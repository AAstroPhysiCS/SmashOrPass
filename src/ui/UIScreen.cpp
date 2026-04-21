#include "smashorpass/ui/UIScreen.hpp"

#include "smashorpass/core/Base.hpp"

#include "spdlog/spdlog.h"

#include "smashorpass/rendering/Renderer.hpp"

namespace sop {
    
    UIScreen::UIScreen(EventDispatcher& dispatcher) 
        : m_EventDispatcher(dispatcher) 
    {
    }

    void UIScreen::OnEvent(const Event& event) 
    {
        EventDispatcher::Dispatch<MouseButtonEvent>(event, [this](const MouseButtonEvent& e) {
            if (!e.Down)
                return;
            Vec2 mousePos{e.X, e.Y};

            for (const UIWidget& w : m_Widgets) {
                if (w.Kind != WidgetKind::Button)
                    continue;

                if (!PointInRect(mousePos, w.LayoutRect))
                    continue;

                const auto& d = std::get<ButtonData>(w.Data);
                if (d.OnClick)
                    d.OnClick(m_EventDispatcher);
            }
        });

        EventDispatcher::Dispatch<MouseMovedEvent>(event, [this](const MouseMovedEvent& e) {
            Vec2 mousePos{e.X, e.Y};

            for (const UIWidget& w : m_Widgets) {
                if (w.Kind != WidgetKind::Button)
                    continue;

                const bool hover = PointInRect(mousePos, w.LayoutRect);
                const auto& d = std::get<ButtonData>(w.Data);
                if (hover && d.OnHover)
                    d.OnHover(m_EventDispatcher);
            }
        });
    }

    void UIScreen::OnUpdate() 
    {
    }

    void UIScreen::OnRender(Renderer& renderer)
    {
        if (m_Root == g_InvalidWidgetId)
            return;

        const SDL_Point outputSize = renderer.GetCurrentOutputSize();

        MeasureWidget(m_Root);
        LayoutWidget(m_Root, SDL_FRect { 0.0f, 0.0f, static_cast<float>(outputSize.x), static_cast<float>(outputSize.y) });

        for (const UIWidget& w : m_Widgets)
            RenderWidget(renderer, w);
    }

    void UIScreen::RenderWidget(Renderer& renderer, const UIWidget& widget) 
    {
        const auto ScaleAroundCenter = [](SDL_FRect r, float scale) {
            const float nw = r.w * scale;
            const float nh = r.h * scale;
            r.x += (r.w - nw) * 0.5f;
            r.y += (r.h - nh) * 0.5f;
            r.w = nw;
            r.h = nh;
            return r;
        };

        switch (widget.Kind) {
            case WidgetKind::Column:
            case WidgetKind::Row:
                break;
            case WidgetKind::Label: {
                const auto& d = std::get<LabelData>(widget.Data);
                renderer.DebugText(
                    widget.LayoutRect.x, widget.LayoutRect.y, d.Text, Color{235, 235, 235, 255});
                break;
            }
            case WidgetKind::Button: {
                const auto& d = std::get<ButtonData>(widget.Data);
                const SDL_FRect buttonRect = widget.LayoutRect;

                renderer.FillRect(buttonRect, Color{52, 58, 64, 255});
                renderer.DrawRect(buttonRect, Color{90, 100, 110, 255});

                constexpr float padX = 14.0f;
                constexpr float padY = 10.0f;

                renderer.DebugText(
                    buttonRect.x + padX, buttonRect.y + padY, d.Text, Color{255, 255, 255, 255});

                break;
            }
        }
    }

    Vec2 UIScreen::MeasureWidget(UIWidgetId id) 
    {
        const auto MeasureText = [](FontId fontId, const std::string& text) -> Vec2 {
            const float charW = (fontId == FontId::Title) ? 12.0f : 8.0f;
            const float lineH = (fontId == FontId::Title) ? 28.0f : 18.0f;
            return Vec2{charW * static_cast<float>(text.size()), lineH};
        };

        UIWidget& w = GetWidgetById(id);

        switch (w.Kind) {
            case WidgetKind::Label: {
                auto& d = std::get<LabelData>(w.Data);
                Vec2 s = MeasureText(d.Font, d.Text);
                w.Measured = SDL_FRect{0, 0, s.x, s.y};
                return s;
            }
            case WidgetKind::Button: {
                constexpr float padX = 14.0f;
                constexpr float padY = 10.0f;

                auto& d = std::get<ButtonData>(w.Data);
                Vec2 text = MeasureText(d.Font, d.Text);
                
                w.Measured = SDL_FRect{0, 0, text.x + padX * 2.0f, text.y + padY * 2.0f};
                return Vec2{w.Measured.w, w.Measured.h};
            }
            case WidgetKind::Column: {
                auto& d = std::get<ColumnData>(w.Data);
                float maxW = 0.0f;
                float totalH = 0.0f;
                int count = 0;

                for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId; c = GetWidgetById(c).NextSibling) {
                    Vec2 cs = MeasureWidget(c);
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

                for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId; c = GetWidgetById(c).NextSibling) {
                    Vec2 cs = MeasureWidget(c);
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
                    Vec2 cs = MeasureWidget(c);
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

                Vec2 childSize = MeasureWidget(w.FirstChild);
                w.Measured = SDL_FRect{0, 0, childSize.x, childSize.y};
                return childSize;
            }
        }

        return Vec2{};
    }

    void UIScreen::LayoutWidget(UIWidgetId id, SDL_FRect rect) 
    {
        UIWidget& w = GetWidgetById(id);
        w.LayoutRect = rect;

        switch (w.Kind) {
            case WidgetKind::Label:
            case WidgetKind::Button:
                return;
            case WidgetKind::Column: {
                auto& d = std::get<ColumnData>(w.Data);
                float y = rect.y;

                for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId; c = GetWidgetById(c).NextSibling) {
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

                for (UIWidgetId c = w.FirstChild; c != g_InvalidWidgetId; c = GetWidgetById(c).NextSibling) {
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

    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    inline Color Lerp(Color a, Color b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return Color{
            static_cast<uint8_t>(Lerp(static_cast<float>(a.r), static_cast<float>(b.r), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.g), static_cast<float>(b.g), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.b), static_cast<float>(b.b), t)),
            static_cast<uint8_t>(Lerp(static_cast<float>(a.a), static_cast<float>(b.a), t)),
        };
    }

/*    void UpdateAnimations(UIScreen& screen, const InputState& input, float dt) {
        for (UIWidget& w : screen.GetWidgets()) {
            if (!w.visual.initialized) {
                w.visual.initialized = true;
                w.visual.rect = w.LayoutRect;
                w.visual.opacity = 1.0f;
            }

            // Layout transition: smooth movement/resize.
            w.visual.rect = SmoothTowards(w.visual.rect, w.LayoutRect, 0.08f, dt);

            if (w.Kind == WidgetKind::Button) {
                const bool hovered = PointInRect(input.mousePos, w.visual.rect);
                const bool pressed = hovered && input.mouseDown;

                w.visual.hoverT = SmoothBool(w.visual.hoverT, hovered, 0.05f, dt);
                w.visual.pressT = SmoothBool(w.visual.pressT, pressed, 0.03f, dt);
            }
        }
    }*/

    void UIScreen::AddChild(UIWidgetId parent, UIWidgetId child) 
    {
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

    void UIScreen::SetRoot(UIWidgetId id) 
    {
        SOP_ASSERT(id < m_Widgets.size(), "Invalid root widget ID");
        m_Root = id;
    }
} // namespace sop
