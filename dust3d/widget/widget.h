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

namespace dust3d
{

class Widget
{
public:
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
        layoutChanged = true;
    }
    
    void setHeightPolicy(SizePolicy heightPolicy)
    {
        if (m_heightPolicy == heightPolicy)
            return;
        m_heightPolicy = heightPolicy;
        layoutChanged = true;
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
        layoutChanged = true;
    }
    
    void setHeight(double height)
    {
        if (Math::isEqual(height, m_height))
            return;
        m_height = height;
        layoutChanged = true;
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
        layoutChanged = true;
    }
    
    void setParent(const Widget *parent)
    {
        if (m_parent == parent)
            return;
        m_parent = parent;
        layoutChanged = true;
    }
    
    void addWidget(std::unique_ptr<Widget> widget)
    {
        widget->setParent(this);
        m_children.push_back(std::move(widget));
        layoutChanged = true;
    }
    
    void addSpacing(double fixedSize)
    {
        auto widget = std::make_unique<Widget>();
        widget->setSizePolicy(SizePolicy::FixedSize);
        widget->setSize(fixedSize, fixedSize);
        addWidget(std::move(widget));
    }
    
    void addExpanding()
    {
        auto widget = std::make_unique<Widget>();
        widget->setSizePolicy(SizePolicy::FlexibleSize);
        addWidget(std::move(widget));
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
    }
    
    void layoutLocation()
    {
        double layoutLeft = m_layoutLeft;
        double layoutTop = m_layoutTop;
        double layoutBottom = m_layoutTop + m_layoutHeight;
        switch (m_layoutDirection) {
        case LayoutDirection::LeftToRight:
            for (auto &widget: m_children) {
                widget->setLayoutLeft(layoutLeft);
                widget->setLayoutTop(layoutTop);
                layoutLeft += widget->layoutWidth();
            }
            break;
        case LayoutDirection::TopToBottom:
            size_t i = 0;
            for (; i < m_children.size(); ++i) {
                auto &widget = m_children[i];
                if (SizePolicy::FlexibleSize == widget->heightPolicy())
                    break;
                widget->setLayoutLeft(layoutLeft);
                widget->setLayoutTop(layoutTop);
                layoutTop += widget->layoutHeight();
            }
            for (int j = (int)m_children.size() - 1; j > i; --j) {
                auto &widget = m_children[j];
                widget->setLayoutLeft(layoutLeft);
                widget->setLayoutTop(layoutBottom - widget->layoutHeight());
                layoutBottom -= widget->layoutHeight();
            }
            break;
        }
        
        for (auto &widget: m_children)
            widget->layoutLocation();
    }

    void layout()
    {
        if (!layoutChanged)
            return;
        
        layoutSize();
        layoutLocation();
        
        layoutChanged = false;
    }
    
    std::vector<std::unique_ptr<Widget>> &children()
    {
        return m_children;
    }
    
    uint64_t renderHints()
    {
        return m_renderHints;
    }
    
protected:
    double m_width = 1.0;
    double m_height = 1.0;
    double m_layoutLeft = 0.0;
    double m_layoutTop = 0.0;
    double m_layoutWidth = 0.0;
    double m_layoutHeight = 0.0;
    bool layoutChanged = true;
    const Widget *m_parent = nullptr;
    uint64_t m_renderHints = 0;
    LayoutDirection m_layoutDirection = LayoutDirection::LeftToRight;
    SizePolicy m_widthPolicy = SizePolicy::RelativeSize;
    SizePolicy m_heightPolicy = SizePolicy::RelativeSize;
    std::vector<std::unique_ptr<Widget>> m_children;
};

}

#endif
