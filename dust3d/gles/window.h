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

#ifndef DUST3D_GLES_WINDOW_H_
#define DUST3D_GLES_WINDOW_H_

#include <dust3d/gles/win32/window_internal.h>

namespace dust3d
{
    
class IndieGameEngine;
    
class Window
{
public:
    enum Type
    {
        Main,
        Popup
    };

    Window(int width, int height, Type type=Type::Main);
    void setVisible(bool visible);
    void setEngine(IndieGameEngine *engine);
    IndieGameEngine *engine() const;
    void addTimer(uint32_t milliseconds, std::function<void (void)> handler);
    WindowInternal &internal();
    static void mainLoop();
    static bool isKeyPressed(char key);
    static uint64_t getMilliseconds();
    void setTitle(const std::string &string);
private:
    WindowInternal m_internal;
    IndieGameEngine *m_engine = nullptr;
};
    
}

#endif