#pragma once

#include "smashorpass/core/Base.hpp"

namespace sop {

    using UIWidgetId = uint64_t;
    inline constexpr UIWidgetId g_InvalidWidgetId = std::numeric_limits<UIWidgetId>::max();

    // maybe change the function signature to pass the widget id or reference to the widget itself,
    // so that the callback can modify it if needed (e.g. for toggle buttons)
    using OnClickFunc = std::function<void()>;
    using OnHoverFunc = std::function<void()>;

    enum class Alignment : uint8_t {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
    };

    enum class HorizontalAlign : uint8_t { 
        Left, 
        Center, 
        Right 
    };

    enum class VerticalAlign : uint8_t { 
        Top, 
        Center, 
        Bottom 
    };

    inline static constexpr HorizontalAlign GetHorizontalAlign(Alignment a) {
        switch (a) {
            case Alignment::TopLeft:
            case Alignment::CenterLeft:
            case Alignment::BottomLeft:
                return HorizontalAlign::Left;
            case Alignment::TopCenter:
            case Alignment::Center:
            case Alignment::BottomCenter:
                return HorizontalAlign::Center;
            case Alignment::TopRight:
            case Alignment::CenterRight:
            case Alignment::BottomRight:
                return HorizontalAlign::Right;
        }

        return HorizontalAlign::Left;
    }

    inline static constexpr VerticalAlign GetVerticalAlign(Alignment a) {
        switch (a) {
            case Alignment::TopLeft:
            case Alignment::TopCenter:
            case Alignment::TopRight:
                return VerticalAlign::Top;
            case Alignment::CenterLeft:
            case Alignment::Center:
            case Alignment::CenterRight:
                return VerticalAlign::Center;
            case Alignment::BottomLeft:
            case Alignment::BottomCenter:
            case Alignment::BottomRight:
                return VerticalAlign::Bottom;
        }

        return VerticalAlign::Top;
    }

    enum class FontId : uint8_t { 
        Body,
        Title,
        Header,
        Big
    };

    enum class WidgetKind : uint8_t {
        Stack,
        Column,
        Row,
        Align,
        Image,
        Label,
        Button,
    };

    struct StackData {};

    struct ColumnData {
        float Spacing = 0.0f;
    };

    struct RowData {
        float Spacing = 0.0f;
    };

    struct AlignData {
        Alignment Alignment = Alignment::Center;
    };

    struct ImageData {
        size_t TextureId = ~0uLL;
    };

    struct LabelData {
        std::string Text;
        FontId FontId = FontId::Body;
    };

    struct ButtonData {
        std::string Text;
        FontId FontId = FontId::Body;
        OnClickFunc OnClick;
        OnHoverFunc OnHover;
    };

    using WidgetData = std::variant<StackData, ColumnData, RowData, AlignData, ImageData, LabelData, ButtonData>;

    struct UIWidget {
        WidgetKind Kind{};
        UIWidgetId Parent = g_InvalidWidgetId;
        UIWidgetId FirstChild = g_InvalidWidgetId;
        UIWidgetId LastChild = g_InvalidWidgetId;
        UIWidgetId NextSibling = g_InvalidWidgetId;

        Alignment SelfAlignment = Alignment::TopLeft;

        WidgetData Data{};
        SDL_FRect Measured{};
        SDL_FRect LayoutRect{};
    };
}