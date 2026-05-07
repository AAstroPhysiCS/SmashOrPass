#include "smashorpass/ui/UIBuilder.hpp"

namespace sop {

void UIBuilder::SetRoot(const NodeBase& node) {
    m_Screen.SetRoot(node.GetId());
}

StackNode UIBuilder::Stack() {
    return StackNode{m_Screen, m_Screen.Create(WidgetKind::Stack, StackData{})};
}

ColumnNode UIBuilder::Column() {
    return ColumnNode{m_Screen, m_Screen.Create(WidgetKind::Column, ColumnData{})};
}

RowNode UIBuilder::Row() {
    return RowNode{m_Screen, m_Screen.Create(WidgetKind::Row, RowData{})};
}

AlignNode UIBuilder::Align(Alignment alignment, const NodeBase& child) {
    AlignNode node(m_Screen, m_Screen.Create(WidgetKind::Align, AlignData{alignment}));
    m_Screen.AddChild(node.GetId(), child.GetId());
    return node;
}

ImageNode UIBuilder::Image(uint64_t textureId) {
    return ImageNode{m_Screen, m_Screen.Create(WidgetKind::Image, ImageData{textureId})};
}

LabelNode UIBuilder::Label(std::string text) {
    return LabelNode{m_Screen, m_Screen.Create(WidgetKind::Label, LabelData{std::move(text), {}})};
}

ButtonNode UIBuilder::Button(std::string text) {
    ButtonData data{};
    data.Text = std::move(text);
    return ButtonNode{m_Screen, m_Screen.Create(WidgetKind::Button, std::move(data))};
}
}  // namespace sop
