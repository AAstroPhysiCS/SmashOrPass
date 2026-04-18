#pragma once

#include "smashorpass/ui/UIWidget.hpp"

#include "smashorpass/core/Event.hpp"
#include "smashorpass/rendering/Renderer.hpp"

namespace sop {

    struct Vec2;

    class UIBuilder;

	class UIScreen {
    public:
        UIScreen() = default;
        virtual ~UIScreen() = default;
        
        inline UIWidget& GetWidgetById(UIWidgetId id) {
            SOP_ASSERT(id < m_Widgets.size(), "Invalid widget ID");
            return m_Widgets[id];
        }

        inline const UIWidget& GetWidgetById(UIWidgetId id) const {
            SOP_ASSERT(id < m_Widgets.size(), "Invalid widget ID");
            return m_Widgets[id];
        }

        inline const std::vector<UIWidget>& GetWidgets() const {
            return m_Widgets;
        }

        virtual void Build(UIBuilder& builder) = 0;

        virtual void OnEvent(const Event& event);
        virtual void OnUpdate();
        virtual void OnRender(Renderer& renderer);
    private:
        Vec2 MeasureWidget(UIWidgetId id);
        void LayoutWidget(UIWidgetId id, SDL_FRect rect);

        void RenderWidget(Renderer& renderer, const UIWidget& widget);

        bool PointInRect(const Vec2& point, const SDL_FRect& rect) const;

        void AddChild(UIWidgetId parent, UIWidgetId child);

        inline UIWidgetId GetRoot() const { return m_Root; }
        void SetRoot(UIWidgetId id);
        
        template <typename TData>
        UIWidgetId Create(WidgetKind kind, TData&& data) {
            // TOOD: maybe make this more efficient by reusing deleted widgets instead of always
            // emplacing back
            UIWidgetId id = static_cast<UIWidgetId>(m_Widgets.size());
            m_Widgets.emplace_back(kind, g_InvalidWidgetId, g_InvalidWidgetId, 
                g_InvalidWidgetId, g_InvalidWidgetId, Alignment::TopLeft, std::forward<TData>(data));
            return id;
        }

        std::vector<UIWidget> m_Widgets;
        UIWidgetId m_Root = g_InvalidWidgetId;

        friend class UIBuilder;

        template <typename TDerived>
        friend struct ContainerNode;
    };
}  // namespace sop