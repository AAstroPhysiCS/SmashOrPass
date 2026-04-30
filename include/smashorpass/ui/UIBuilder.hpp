#pragma once

#include "smashorpass/core/Base.hpp"
#include "smashorpass/ui/UIScreen.hpp"
#include "smashorpass/ui/UIWidget.hpp"

namespace sop {

class NodeBase {
   public:
    NodeBase(UIScreen& screen, UIWidgetId id) : m_Screen(screen), m_Id(id) {}

    inline UIWidgetId GetId() const {
        return m_Id;
    }

    template <typename TSelf>
    auto&& Align(this TSelf&& self, Alignment alignment) {
        self.GetScreen().GetWidgetById(self.GetId()).SelfAlignment = alignment;
        return std::forward<TSelf>(self);
    }

    inline explicit operator bool() const {
        return m_Id != g_InvalidWidgetId;
    }

   protected:
    inline UIScreen& GetScreen() const {
        return m_Screen;
    }

   private:
    UIScreen& m_Screen;
    UIWidgetId m_Id = g_InvalidWidgetId;
};

template <typename TDerived>
struct ContainerNode : public NodeBase {
    using NodeBase::NodeBase;

    template <typename... TChildren>
    TDerived& Add(const TChildren&... children) {
        (this->GetScreen().AddChild(this->GetId(), children.GetId()), ...);
        return static_cast<TDerived&>(*this);
    }
};

struct StackNode : public ContainerNode<StackNode> {
    using ContainerNode<StackNode>::ContainerNode;
};

struct ColumnNode : public ContainerNode<ColumnNode> {
    using ContainerNode<ColumnNode>::ContainerNode;

    ColumnNode& Spacing(float Spacing) {
        auto& data = std::get<ColumnData>(this->GetScreen().GetWidgetById(this->GetId()).Data);
        data.Spacing = Spacing;
        return *this;
    }
};

struct RowNode : public ContainerNode<RowNode> {
    using ContainerNode<RowNode>::ContainerNode;

    RowNode& Spacing(float Spacing) {
        auto& data = std::get<RowData>(this->GetScreen().GetWidgetById(this->GetId()).Data);
        data.Spacing = Spacing;
        return *this;
    }
};

struct ImageNode : public NodeBase {
    using NodeBase::NodeBase;
};

struct AlignNode : public ContainerNode<AlignNode> {
    using ContainerNode<AlignNode>::ContainerNode;
};

struct LabelNode : public NodeBase {
    using NodeBase::NodeBase;

    LabelNode& Font(FontId fontId) {
        auto& data = std::get<LabelData>(this->GetScreen().GetWidgetById(this->GetId()).Data);
        data.Font = fontId;
        return *this;
    }
};

struct ButtonNode : public NodeBase {
    using NodeBase::NodeBase;

    template <typename TFn>
    ButtonNode& OnClick(TFn&& callback) {
        auto& data = std::get<ButtonData>(this->GetScreen().GetWidgetById(this->GetId()).Data);
        data.OnClick = std::forward<TFn>(callback);
        return *this;
    }

    template <typename TFn>
    ButtonNode& OnHover(TFn&& callback) {
        auto& data = std::get<ButtonData>(this->GetScreen().GetWidgetById(this->GetId()).Data);
        data.OnHover = std::forward<TFn>(callback);
        return *this;
    }
};

class UIBuilder {
   public:
    explicit UIBuilder(UIScreen& screen) : m_Screen(screen) {}

    void SetRoot(const NodeBase& node);

    StackNode Stack();
    ColumnNode Column();
    RowNode Row();
    AlignNode Align(Alignment alignment, const NodeBase& child);
    ImageNode Image(uint64_t textureId);
    LabelNode Label(std::string text);
    ButtonNode Button(std::string text);

   private:
    UIScreen& m_Screen;
};
}  // namespace sop
