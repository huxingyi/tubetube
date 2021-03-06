/*
 *  Copyright (c) 2022 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef HU_WIDGET_WIDGET_H_
#define HU_WIDGET_WIDGET_H_

#include <format>
#include <map>
#include <vector>
#include <hu/base/color.h>
#include <hu/base/math.h>
#include <hu/base/signal.h>

namespace Hu
{

class Widget
{
public:

    class Window
    {
    public:
        bool layoutChanged() const
        {
            return m_layoutChanged;
        }
        
        virtual ~Window()
        {
        }
        
        bool appearanceChanged() const
        {
            return m_appearanceChanged;
        }
        
        void setLayoutChanged(bool changed)
        {
            if (m_layoutChanged == changed)
                return;
            m_layoutChanged = changed;
        }
        
        void setAppearanceChanged(bool changed)
        {
            if (m_appearanceChanged == changed)
                return;
            m_appearanceChanged = changed;
        }
        
        void setName(const std::string &name)
        {
            if (m_name == name)
                return;
            m_name = name;
        }
        
        std::map<std::string, Widget *> &widgets()
        {
            return m_widgets;
        }
        
        Widget *getWidget(const std::string &id) const
        {
            auto findWidget = m_widgets.find(id);
            if (findWidget == m_widgets.end())
                return nullptr;
            return findWidget->second;
        }
        
        const std::string &name() const
        {
            return m_name;
        }
        
        virtual void update() = 0;
        
    private:
        bool m_layoutChanged = false;
        bool m_appearanceChanged = false;
        std::map<std::string, Widget *> m_widgets;
        std::string m_name;
    };

    Signal<double, double> mouseMoved;
    Signal<> mouseEntered;
    Signal<> mouseLeaved;
    Signal<> mousePressed;
    Signal<> mouseReleased;

    Widget(Window *window, const std::string &id=std::string()):
        m_window(window),
        m_id(id)
    {
        if (m_id.empty())
            m_id = std::format("{}_unamed", m_nextWidgetId++);
        
        m_window->widgets().insert({m_id, this});
    }
    
    const std::string &id() const
    {
        return m_id;
    }
    
    virtual ~Widget()
    {
    }
    
    virtual double minimalLayoutWidth()
    {
        return 0.0;
    }
    
    virtual double minimalLayoutHeight()
    {
        return 0.0;
    }

    enum LayoutDirection
    {
        LeftToRight = 0,
        TopToBottom
    };
    
    enum SizePolicy
    {
        FixedSize = 0x00000001,
        RelativeSize = 0x00000002,
        FlexibleSize = 0x00000004,
        MinimalSize = 0x00000008,
    };
    
    enum RenderHint
    {
        Container = 0x00000001,
        Element = 0x00000002,
        PushButton = 0x00000004,
        RadioButton = 0x00000008,
        Text = 0x00000010,
        Canvas = 0x00000020,
    };
    
    std::string SizePolicyToString(SizePolicy sizePolicy)
    {
        std::string string;
        if (sizePolicy & FixedSize)
            string += "FixedSize ";
        if (sizePolicy & RelativeSize)
            string += "RelativeSize ";
        if (sizePolicy & FlexibleSize)
            string += "FlexibleSize ";
        if (sizePolicy & MinimalSize)
            string += "MinimalSize ";
        return string;
    }

    const double &width() const
    {
        return m_width;
    }
    
    const double &height() const
    {
        return m_height;
    }
    
    void setWidthPolicy(SizePolicy widthPolicy)
    {
        if (m_widthPolicy == widthPolicy)
            return;
        m_widthPolicy = widthPolicy;
        m_window->setLayoutChanged(true);
    }
    
    SizePolicy widthPolicy() const
    {
        return m_widthPolicy;
    }
    
    void setHeightPolicy(SizePolicy heightPolicy)
    {
        if (m_heightPolicy == heightPolicy)
            return;
        m_heightPolicy = heightPolicy;
        m_window->setLayoutChanged(true);
    }
    
    SizePolicy heightPolicy() const
    {
        return m_heightPolicy;
    }
    
    void setSizePolicy(SizePolicy sizePolicy)
    {
        setWidthPolicy(sizePolicy);
        setHeightPolicy(sizePolicy);
    }
    
    void setWidth(double size, SizePolicy sizePolicy)
    {
        setWidthPolicy(sizePolicy);
        setWidth(size);
    }
    
    void setHeight(double size, SizePolicy sizePolicy)
    {
        setHeightPolicy(sizePolicy);
        setHeight(size);
    }
    
    void setWidth(double width)
    {
        if (Math::isEqual(width, m_width))
            return;
        m_width = width;
        m_window->setLayoutChanged(true);
    }
    
    void setHeight(double height)
    {
        if (Math::isEqual(height, m_height))
            return;
        m_height = height;
        m_window->setLayoutChanged(true);
    }
    
    void setSize(double width, double height)
    {
        setWidth(width);
        setHeight(height);
    }
    
    void setSize(double size)
    {
        setSize(size, size);
    }
    
    void setLayoutDirection(LayoutDirection layoutDirection)
    {
        if (m_layoutDirection == layoutDirection)
            return;
        m_layoutDirection = layoutDirection;
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            setHeightPolicy(SizePolicy::MinimalSize);
            break;
        case LayoutDirection::TopToBottom:
            setWidthPolicy(SizePolicy::MinimalSize);
            break;
        }
        m_window->setLayoutChanged(true);
    }
    
    void setParent(const Widget *parent)
    {
        if (m_parent == parent)
            return;
        m_parent = parent;
        m_window->setLayoutChanged(true);
    }
    
    void addWidget(Widget *widget)
    {
        widget->setParent(this);
        m_children.push_back(widget);
        m_window->setLayoutChanged(true);
    }
    
    void addSpacing(double fixedSize)
    {
        auto widget = new Widget(m_window);
        widget->setName(std::format("Spacing {}", fixedSize));
        widget->setSizePolicy(SizePolicy::FixedSize);
        widget->setSize(fixedSize, fixedSize);
        addWidget(widget);
    }
    
    void addExpanding(double weight=1.0)
    {
        auto widget = new Widget(m_window);
        widget->setName(std::format("Expanding {}", weight));
        widget->setSizePolicy(SizePolicy::FlexibleSize);
        widget->setSize(weight);
        addWidget(widget);
    }
    
    double layoutLeft() const
    {
        return m_layoutLeft;
    }
    
    void setLayoutLeft(double left)
    {
        m_layoutLeft = left;
    }
    
    double layoutTop() const
    {
        return m_layoutTop;
    }
    
    void setLayoutTop(double top)
    {
        m_layoutTop = top;
    }
    
    double layoutWidth() const
    {
        return m_layoutWidth;
    }
    
    void setLayoutWidth(double width)
    {
        m_layoutWidth = width;
    }
    
    double layoutHeight() const
    {
        return m_layoutHeight;
    }
    
    void setLayoutHeight(double height)
    {
        m_layoutHeight = height;
    }
    
    double parentLayoutWidth() const
    {
        if (nullptr == m_parent)
            return 0;
        return m_parent->layoutWidth();
    }
    
    double parentLayoutHeight() const
    {
        if (nullptr == m_parent)
            return 0;
        return m_parent->layoutHeight();
    }
    
    void layoutExpandingChildrenSize(size_t depth)
    {
        double usedSize = 0.0;
        double totalExpandingWeights = 0.0;
        
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutExpandingChildrenSize [" << name() << "] LeftToRight layoutWidth:" << layoutWidth() << std::endl;
            for (const auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->widthPolicy()) {
                    totalExpandingWeights += widget->width();
                    continue;
                }
                if (m_debugLayoutEnabled)
                    std::cout << std::string(depth * 4, ' ') << "    [" << widget->name() << "] layoutWidth:" << widget->layoutWidth() << " widthPolicy:" << SizePolicyToString(widget->widthPolicy()) << std::endl;
                usedSize += widget->layoutWidth();
            }
            break;
        case LayoutDirection::TopToBottom:
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutExpandingChildrenSize [" << name() << "] TopToBottom layoutHeight:" << layoutHeight() << std::endl;
            for (const auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->heightPolicy()) {
                    totalExpandingWeights += widget->height();
                    continue;
                }
                if (m_debugLayoutEnabled)
                    std::cout << std::string(depth * 4, ' ') << "    [" << widget->name() << "] layoutHeight:" << widget->layoutHeight() << " heightPolicy:" << SizePolicyToString(widget->heightPolicy()) << std::endl;
                usedSize += widget->layoutHeight();
            }
            break;
        }
        if (Math::isZero(totalExpandingWeights))
            return;
        
        double expandingSize = 0.0;
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            expandingSize = (m_layoutWidth - usedSize) / totalExpandingWeights;
            for (auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->widthPolicy()) {
                    if (m_debugLayoutEnabled)
                        std::cout << std::string(depth * 4, ' ') << "set child width [" << name() << "]:" << (expandingSize * widget->width()) << std::endl;
                    widget->setLayoutWidth(expandingSize * widget->width());
                }
            }
            break;
        case LayoutDirection::TopToBottom:
            expandingSize = (m_layoutHeight - usedSize) / totalExpandingWeights;
            for (auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->heightPolicy()) {
                    if (m_debugLayoutEnabled)
                        std::cout << std::string(depth * 4, ' ') << "set child height [" << name() << "]:" << (expandingSize * widget->height()) << std::endl;
                    widget->setLayoutHeight(expandingSize * widget->height());
                }
            }
            break;
        }
    }
    
    void layoutSizeFixed(size_t depth)
    {
        switch (m_widthPolicy) {
        case SizePolicy::FixedSize:
            m_layoutWidth = m_width;
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeFixed    [" << name() << "] m_layoutWidth:" << m_layoutWidth << " widthPolicy:" << SizePolicyToString(widthPolicy()) << std::endl;
            break;
        }
        
        switch (m_heightPolicy) {
        case SizePolicy::FixedSize:
            m_layoutHeight = m_height;
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeFixed    [" << name() << "] m_layoutHeight:" << m_layoutHeight << " heightPolicy:" << SizePolicyToString(heightPolicy()) << std::endl;
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutSizeFixed(depth + 1);
    }
    
    void layoutSizeBottomUp(size_t depth)
    {
        for (auto &widget: m_children)
            widget->layoutSizeBottomUp(depth + 1);
        
        switch (m_widthPolicy) {
        case SizePolicy::MinimalSize:
            m_layoutWidth = minimalLayoutWidth();
            for (auto &widget: m_children)
                m_layoutWidth = std::max(m_layoutWidth, widget->layoutWidth());
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeBottomUp    [" << name() << "] m_layoutWidth:" << m_layoutWidth << " widthPolicy:" << SizePolicyToString(widthPolicy()) << std::endl;
            break;
        }
        
        switch (m_heightPolicy) {
        case SizePolicy::MinimalSize:
            m_layoutHeight = minimalLayoutHeight();
            for (auto &widget: m_children)
                m_layoutHeight = std::max(m_layoutHeight, widget->layoutHeight());
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeBottomUp    [" << name() << "] m_layoutHeight:" << m_layoutHeight << " heightPolicy:" << SizePolicyToString(heightPolicy()) << std::endl;
            break;
        }
    }
    
    void layoutSizeTopDown(size_t depth)
    {
        switch (m_widthPolicy) {
        case SizePolicy::RelativeSize:
            m_layoutWidth = parentLayoutWidth() * m_width;
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeTopDown    [" << name() << "] m_layoutWidth:" << m_layoutWidth << " widthPolicy:" << SizePolicyToString(widthPolicy()) << " parentLayoutWidth:" << parentLayoutWidth() << std::endl;
            break;
        }
        
        switch (m_heightPolicy) {
        case SizePolicy::RelativeSize:
            m_layoutHeight = parentLayoutHeight() * m_height;
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "layoutSizeTopDown    [" << name() << "] m_layoutHeight:" << m_layoutHeight << " heightPolicy:" << SizePolicyToString(heightPolicy()) << " parentLayoutHeight:" << parentLayoutHeight() << std::endl;
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutSizeTopDown(depth + 1);
        
        layoutExpandingChildrenSize(depth);
    }
    
    void layoutLocation(size_t depth)
    {
        if (m_children.empty())
            return;
        
        double layoutLeft = m_layoutLeft;
        double layoutTop = m_layoutTop;
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "[" << name() << "] LeftToRight:" << std::endl;
            for (auto &widget: m_children) {
                widget->setLayoutTop(layoutTop);
                widget->setLayoutLeft(layoutLeft);
                if (m_debugLayoutEnabled)
                    std::cout << std::string(depth * 4, ' ') << "    [" << widget->name() << "] layoutWidth:" << widget->layoutWidth() << " layoutHeight:" << widget->layoutHeight() << std::endl;
                layoutLeft += widget->layoutWidth();
            }
            break;
        case LayoutDirection::TopToBottom:
            if (m_debugLayoutEnabled)
                std::cout << std::string(depth * 4, ' ') << "[" << name() << "] TopToBottom:" << std::endl;
            for (auto &widget: m_children) {
                widget->setLayoutLeft(layoutLeft);
                widget->setLayoutTop(layoutTop);
                if (m_debugLayoutEnabled)
                    std::cout << std::string(depth * 4, ' ') << "    [" << widget->name() << "] layoutHeight:" << widget->layoutHeight() << " layoutWidth:" << widget->layoutWidth() << " heightPolicy:" << SizePolicyToString(widget->heightPolicy()) << std::endl;
                layoutTop += widget->layoutHeight();
            }
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutLocation(depth + 1);
    }

    void layout(size_t depth=0)
    {
        if (m_debugLayoutEnabled)
            std::cout << std::string(depth * 4, ' ') << "layout begin name:" << name() << std::endl;
        layoutSizeFixed(depth);
        layoutSizeBottomUp(depth);
        layoutSizeTopDown(depth);
        layoutSizeTopDown(depth); // Relayout because of parent widget may resized
        layoutLocation(depth);
        if (m_debugLayoutEnabled)
            std::cout << std::string(depth * 4, ' ') << "layout end name:" << name() << " layoutSize:" << layoutWidth() << "," << layoutHeight() << std::endl;
    }
    
    void setMouseHovering(bool hovering)
    {
        if (m_mouseHovering == hovering)
            return;
        m_mouseHovering = hovering;
        if (m_mouseHovering)
            mouseEntered.emit();
        else
            mouseLeaved.emit();
    }
    
    void setMousePressing(bool pressing)
    {
        if (m_mousePressing == pressing)
            return;
        m_mousePressing = pressing;
        if (!m_mouseHovering)
            return;
        if (m_mousePressing)
            mousePressed.emit();
        else
            mouseReleased.emit();
    }
    
    bool handleMouseMove(double x, double y)
    {
        if (x >= m_layoutLeft && x < m_layoutLeft + m_layoutWidth &&
                y >= m_layoutTop && y < m_layoutTop + m_layoutHeight) {
            setMouseHovering(true);
            for (auto &widget: m_children)
                widget->handleMouseMove(x, y);
            mouseMoved.emit(x, y);
            return true;
        }
        for (auto &widget: m_children)
            widget->handleMouseMove(x, y);
        setMouseHovering(false);
        return false;
    }
    
    void handleMouseLeftButtonDown()
    {
        if (m_mouseHovering)
            setMousePressing(true);
        for (auto &widget: m_children)
            widget->handleMouseLeftButtonDown();
    }
    
    void handleMouseLeftButtonUp()
    {
        for (auto &widget: m_children)
            widget->handleMouseLeftButtonUp();
        setMousePressing(false);
    }
    
    std::vector<Widget *> &children()
    {
        return m_children;
    }
    
    const Widget *parent() const
    {
        return m_parent;
    }
    
    const Color &parentColor() const
    {
        const Widget *parent = m_parent;
        while (nullptr != parent && parent->parent()) {
            if (parent->backgroundColor().alpha() > 0)
                break;
            parent = parent->parent();
        }
        if (nullptr == parent)
            return m_backgroundColor;
        return parent->backgroundColor();
    }
    
    uint64_t renderHints() const
    {
        return m_renderHints;
    }
    
    const Color &backgroundColor() const
    {
        return m_backgroundColor;
    }
    
    void setBackgroundColor(const Color &color)
    {
        if (m_backgroundColor == color)
            return;
        m_backgroundColor = color;
        m_window->setAppearanceChanged(true);
    }
    
    const Color &color() const
    {
        return m_color;
    }
    
    void setColor(const Color &color)
    {
        if (m_color == color)
            return;
        m_color = color;
        m_window->setAppearanceChanged(true);
    }
    
    const std::string &backgroundImageResourceName() const
    {
        return m_backgroundImageResourceName;
    }
    
    void setBackgroundImageResourceName(const std::string &name)
    {
        if (m_backgroundImageResourceName == name)
            return;
        m_backgroundImageResourceName = name;
        m_window->setAppearanceChanged(true);
    }
    
    const double &backgroundImageOpacity() const
    {
        return m_backgroundImageOpacity;
    }
    
    void setBackgroundImageOpacity(double opacity)
    {
        if (Math::isEqual(m_backgroundImageOpacity, opacity))
            return;
        m_backgroundImageOpacity = opacity;
        m_window->setAppearanceChanged(true);
    }
    
    const std::string &name() const
    {
        return m_name;
    }
    
    void setName(const std::string &name)
    {
        if (m_name == name)
            return;
        m_name = name;
        m_window->setAppearanceChanged(true);
    }
    
    void setPadding(double left, double top, double right, double bottom)
    {
        if (Math::isEqual(left, m_paddingLeft) &&
                Math::isEqual(top, m_paddingTop) &&
                Math::isEqual(right, m_paddingRight) &&
                Math::isEqual(bottom, m_paddingBottom))
            return;
        m_paddingLeft = left;
        m_paddingTop = top;
        m_paddingRight = right;
        m_paddingBottom = bottom;
        m_window->setAppearanceChanged(true);
    }
    
    double paddingLeft() const
    {
        return m_paddingLeft;
    }
    
    double paddingTop() const
    {
        return m_paddingTop;
    }
    
    double paddingRight() const
    {
        return m_paddingRight;
    }
    
    double paddingBottom() const
    {
        return m_paddingBottom;
    }
    
    double paddingWidth() const
    {
        return m_paddingLeft + m_paddingRight;
    }
    
    double paddingHeight() const
    {
        return m_paddingTop + m_paddingBottom;
    }

    static uint64_t m_nextWidgetId;
    static const bool m_debugLayoutEnabled;
protected:
    Window *m_window = nullptr;
    double m_width = 1.0;
    double m_height = 0.0;
    double m_layoutLeft = 0.0;
    double m_layoutTop = 0.0;
    double m_layoutWidth = 0.0;
    double m_layoutHeight = 0.0;
    double m_paddingLeft = 0.0;
    double m_paddingTop = 0.0;
    double m_paddingRight = 0.0;
    double m_paddingBottom = 0.0;
    bool m_mouseHovering = false;
    bool m_mousePressing = false;
    const Widget *m_parent = nullptr;
    uint64_t m_renderHints = 0;
    Color m_backgroundColor;
    Color m_color;
    std::string m_name;
    std::string m_backgroundImageResourceName;
    double m_backgroundImageOpacity = 1.0;
    LayoutDirection m_layoutDirection = LayoutDirection::LeftToRight;
    SizePolicy m_widthPolicy = SizePolicy::RelativeSize;
    SizePolicy m_heightPolicy = SizePolicy::MinimalSize;
    std::vector<Widget *> m_children;
    std::string m_id;
};

}

#endif
