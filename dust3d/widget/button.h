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

#ifndef DUST3D_WIDGET_BUTTON_H_
#define DUST3D_WIDGET_BUTTON_H_

#include <dust3d/widget/widget.h>

namespace dust3d
{
    
class Button: public Widget
{
public:
    Button()
    {
        m_heightPolicy = SizePolicy::FixedSize;
        m_height = 16.0;
        m_widthPolicy = SizePolicy::FixedSize;
        m_width = 600.0;
        m_renderHints = RenderHint::Element | RenderHint::Button;
    }
    
    void setIcon(const std::string &icon)
    {
        if (m_icon == icon)
            return;
        m_icon = icon;
    }
    
    void setText(const std::string &text)
    {
        if (m_text == text)
            return;
        m_text = text;
    }
    
    const std::string &icon()
    {
        return m_icon;
    }
    
    const std::string &text()
    {
        return m_text;
    }
    
private:
    std::string m_icon;
    std::string m_text;
};

}

#endif

