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

#ifndef HU_WIDGET_RADIO_BUTTON_H_
#define HU_WIDGET_RADIO_BUTTON_H_

#include <hu/widget/widget.h>

namespace Hu
{
    
class RadioButton: public Widget
{
public:
    RadioButton()
    {
        m_renderHints = RenderHint::Element | RenderHint::RadioButton;
    }
    
    void setText(const std::string &text)
    {
        if (m_text == text)
            return;
        m_text = text;
        m_appearanceChanged = true;
    }
    
    const std::string &text()
    {
        return m_text;
    }
    
    bool checked() const
    {
        return m_checked;
    }
    
    void setChecked(bool checked)
    {
        if (m_checked == checked)
            return;
        m_checked = checked;
        m_appearanceChanged = true;
    }
    
private:
    std::string m_text;
    bool m_checked = false;
};

}

#endif
