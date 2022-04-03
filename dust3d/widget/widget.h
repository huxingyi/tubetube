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

#ifndef DUST3D_WIDGET_WIDGET_H_
#define DUST3D_WIDGET_WIDGET_H_

#include <dust3d/base/math.h>
#include <dust3d/base/color.h>

namespace dust3d
{

class Widget
{
public:
    Widget()
    {
    }
    
    virtual ~Widget()
    {
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
    };
    
    enum RenderHint
    {
        Container = 0x00000001,
        Element = 0x00000002,
        Button = 0x00000004,
    };

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
        m_layoutChanged = true;
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
        m_layoutChanged = true;
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
    
    void setWidth(double width)
    {
        if (Math::isEqual(width, m_width))
            return;
        m_width = width;
        m_layoutChanged = true;
    }
    
    void setHeight(double height)
    {
        if (Math::isEqual(height, m_height))
            return;
        m_height = height;
        m_layoutChanged = true;
    }
    
    void setSize(double width, double height)
    {
        setWidth(width);
        setHeight(height);
    }
    
    void setLayoutDirection(LayoutDirection layoutDirection)
    {
        if (m_layoutDirection == layoutDirection)
            return;
        m_layoutDirection = layoutDirection;
        m_layoutChanged = true;
    }
    
    void setParent(const Widget *parent)
    {
        if (m_parent == parent)
            return;
        m_parent = parent;
        m_layoutChanged = true;
    }
    
    void addWidget(std::unique_ptr<Widget> widget)
    {
        widget->setParent(this);
        m_children.push_back(std::move(widget));
        m_layoutChanged = true;
    }
    
    void addSpacing(double fixedSize)
    {
        auto widget = std::make_unique<Widget>();
        widget->setSizePolicy(SizePolicy::FixedSize);
        widget->setSize(fixedSize, fixedSize);
        addWidget(std::move(widget));
    }
    
    void addExpanding(double weight=1.0)
    {
        auto widget = std::make_unique<Widget>();
        widget->setSizePolicy(SizePolicy::FlexibleSize);
        widget->setExpandingWeight(weight);
        addWidget(std::move(widget));
    }
    
    double expandingWeight() const
    {
        return m_expandingWeight;
    }
    
    void setExpandingWeight(double weight)
    {
        if (Math::isEqual(m_expandingWeight, weight))
            return;
        m_expandingWeight = weight;
        m_layoutChanged = true;
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
    
    void layoutExpandingChildrenSize()
    {
        double usedSize = 0.0;
        double totalExpandingWeights = 0.0;
        
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            for (const auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->widthPolicy()) {
                    totalExpandingWeights += widget->expandingWeight();
                    continue;
                }
                usedSize += widget->layoutWidth();
            }
            break;
        case LayoutDirection::TopToBottom:
            for (const auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->heightPolicy()) {
                    totalExpandingWeights += widget->expandingWeight();
                    continue;
                }
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
                if (SizePolicy::FlexibleSize == widget->widthPolicy())
                    widget->setLayoutWidth(expandingSize * widget->expandingWeight());
            }
            break;
        case LayoutDirection::TopToBottom:
            expandingSize = (m_layoutHeight - usedSize) / totalExpandingWeights;
            for (auto &widget: m_children) {
                if (SizePolicy::FlexibleSize == widget->heightPolicy())
                    widget->setLayoutHeight(expandingSize * widget->expandingWeight());
            }
            break;
        }
    }
    
    void layoutSize()
    {
        switch (m_widthPolicy) {
        case SizePolicy::FixedSize:
            m_layoutWidth = m_width;
            break;
        case SizePolicy::RelativeSize:
            m_layoutWidth = parentLayoutWidth() * m_width;
            break;
        }
        
        switch (m_heightPolicy) {
        case SizePolicy::FixedSize:
            m_layoutHeight = m_height;
            break;
        case SizePolicy::RelativeSize:
            m_layoutHeight = parentLayoutHeight() * m_height;
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutSize();
        
        layoutExpandingChildrenSize();
    }
    
    void layoutLocation()
    {
        double layoutLeft = m_layoutLeft;
        double layoutTop = m_layoutTop;
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            for (auto &widget: m_children) {
                widget->setLayoutTop(layoutTop);
                widget->setLayoutLeft(layoutLeft);
                layoutLeft += widget->layoutWidth();
            }
            break;
        case LayoutDirection::TopToBottom:
            for (auto &widget: m_children) {
                widget->setLayoutLeft(layoutLeft);
                widget->setLayoutTop(layoutTop);
                layoutTop += widget->layoutHeight();
            }
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutLocation();
    }

    void layout()
    {
        if (!m_layoutChanged)
            return;
        
        layoutSize();
        layoutLocation();
        
        m_layoutChanged = false;
    }
    
    std::vector<std::unique_ptr<Widget>> &children()
    {
        return m_children;
    }
    
    uint64_t renderHints()
    {
        return m_renderHints;
    }
    
    const Color &backgroundColor()
    {
        return m_backgroundColor;
    }
    
    void setBackgroundColor(const Color &color)
    {
        if (m_backgroundColor == color)
            return;
        m_backgroundColor = color;
    }
    
    const Color &color()
    {
        return m_color;
    }
    
    void setColor(const Color &color)
    {
        if (m_color == color)
            return;
        m_color = color;
    }
    
    bool layoutChanged() const
    {
        return m_layoutChanged;
    }
    
protected:
    double m_width = 1.0;
    double m_height = 1.0;
    double m_layoutLeft = 0.0;
    double m_layoutTop = 0.0;
    double m_layoutWidth = 0.0;
    double m_layoutHeight = 0.0;
    double m_expandingWeight = 0.0;
    bool m_layoutChanged = true;
    const Widget *m_parent = nullptr;
    uint64_t m_renderHints = 0;
    Color m_backgroundColor;
    Color m_color;
    LayoutDirection m_layoutDirection = LayoutDirection::LeftToRight;
    SizePolicy m_widthPolicy = SizePolicy::RelativeSize;
    SizePolicy m_heightPolicy = SizePolicy::RelativeSize;
    std::vector<std::unique_ptr<Widget>> m_children;
};

}

#endif
