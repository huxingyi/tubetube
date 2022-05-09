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

#ifndef HU_GLES_WINDOW_H_
#define HU_GLES_WINDOW_H_

#include <hu/gles/win32/window_internal.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>

namespace Hu
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

    Window(int width, int height, Type type=Type::Main, Window *parent=nullptr);
    void setVisible(bool visible);
    void setEngine(IndieGameEngine *engine);
    IndieGameEngine *engine() const;
    EGLDisplay eglDisplay() const;
    EGLDisplay eglSurface() const;
    EGLDisplay eglContext() const;
    void addTimer(uint32_t milliseconds, std::function<void (void)> handler);
    WindowInternal &internal();
    void setTitle(const std::string &string);
    void setPosition(int x, int y);
    Type type() const;
    Window *popupWindow();
    static void mainLoop();
    static bool isKeyPressed(char key);
    static uint64_t getMilliseconds();
    static std::pair<int, int> getCursorPosition();
private:
    WindowInternal m_internal;
    IndieGameEngine *m_engine = nullptr;
    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_eglSurface = EGL_NO_SURFACE;
    EGLContext m_eglContext = EGL_NO_CONTEXT;
    Type m_type = Type::Main;
    Window *m_popupWindow = nullptr;
};
    
}

#endif
