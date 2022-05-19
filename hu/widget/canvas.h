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

#ifndef HU_WIDGET_CANVAS_H_
#define HU_WIDGET_CANVAS_H_

#include <hu/base/color.h>
#include <hu/widget/widget.h>

namespace Hu
{

class Canvas: public Widget
{
public:
    struct Line
    {
        double fromX;
        double fromY;
        double toX;
        double toY;
        Color color;
    };
    
    struct Rectangle
    {
        double left;
        double top;
        double right;
        double bottom;
        Color color;
    };
    
    Canvas()
    {
        m_renderHints = RenderHint::Canvas;
    }
    
    void addLine(double fromX, double fromY, double toX, double toY, const Color &color)
    {
        m_lines.push_back({fromX, fromY, toX, toY, color});
        m_appearanceChanged = true;
    }
    
    void addRectangle(double left, double top, double right, double bottom, const Color &color)
    {
        m_rectangles.push_back({left, top, right, bottom, color});
        m_appearanceChanged = true;
    }
    
    const std::vector<Line> &lines() const
    {
        return m_lines;
    }
    
    const std::vector<Rectangle> &rectangles() const
    {
        return m_rectangles;
    }
    
private:
    std::vector<Line> m_lines;
    std::vector<Rectangle> m_rectangles;
};

}

#endif
