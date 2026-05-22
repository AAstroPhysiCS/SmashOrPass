#pragma once

#include "smashorpass/core/Base.hpp"
#include "smashorpass/core/Event.hpp"
#include "smashorpass/ui/UIWidget.hpp"

namespace sop {

struct AppCtx;
class UIBuilder;

class UIScreen {
   public:
    explicit UIScreen(AppCtx& ctx);
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

    virtual EventFlow OnEvent(AppCtx& ctx, const Event& event);
    virtual void OnUpdate(AppCtx& ctx);
    virtual Result<void> OnRender(AppCtx& ctx);

    void RebuildUI();
   protected:
       inline AppCtx& GetAppCtx() { return m_Ctx; }
   private:
    Result<Vec2> MeasureWidget(AppCtx& ctx, UIWidgetId id);
    void LayoutWidget(UIWidgetId id, SDL_FRect rect);

    Result<void> RenderWidget(AppCtx& ctx, const UIWidget& widget);

    bool PointInRect(const Vec2& point, const SDL_FRect& rect) const;

    void AddChild(UIWidgetId parent, UIWidgetId child);

    inline UIWidgetId GetRoot() const {
        return m_Root;
    }
    void SetRoot(UIWidgetId id);

    template <typename TData>
    UIWidgetId Create(WidgetKind kind, TData&& data) {
        // TOOD: maybe make this more efficient by reusing deleted widgets instead of always
        // emplacing back
        UIWidgetId id = static_cast<UIWidgetId>(m_Widgets.size());
        m_Widgets.emplace_back(kind,
                               g_InvalidWidgetId,
                               g_InvalidWidgetId,
                               g_InvalidWidgetId,
                               g_InvalidWidgetId,
                               Alignment::TopLeft,
                               std::forward<TData>(data));
        return id;
    }

    bool m_RebuildRequested = false;

    std::vector<UIWidget> m_Widgets;
    UIWidgetId m_Root = g_InvalidWidgetId;
    AppCtx& m_Ctx;

    friend class UIBuilder;

    template <typename TDerived>
    friend struct ContainerNode;
};
}  // namespace sop
