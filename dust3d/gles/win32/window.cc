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

#define NOMINMAX
#include <locale>
#include <codecvt>
#include <dust3d/gles/indie_game_engine.h>
#include <dust3d/gles/window.h>

namespace dust3d
{

static int g_windowCount = 0;

static LRESULT CALLBACK windowMessageHandler(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
    Window *window = (Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams); 
            ++g_windowCount;
        }
        break;
    case WM_DESTROY: {
            --g_windowCount;
            if (0 == g_windowCount)
                PostQuitMessage(0);
        }
        break;
    case WM_CLOSE : {
            DestroyWindow(hwnd);
            return 0;
        } 
        break;
    case WM_SIZE: {
            auto windowWidth = LOWORD(lParam);
            auto windowHeight = HIWORD(lParam);
            if (nullptr != window && nullptr != window->engine())
                window->engine()->setWindowSize(windowWidth, windowHeight);
            return 0;
        } 
        break;
    case WM_MOUSEWHEEL: {
            //auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            //fov += zDelta > 0.0 ? -5.0 : 5.0;
            //if (fov < 1.0f)
            //    fov = 1.0f;
            //if (fov > 45.0f)
            //    fov = 45.0f; 
        } 
        break;
    case WM_MOUSEMOVE: {
            auto x = GET_X_LPARAM(lParam);
            auto y = GET_Y_LPARAM(lParam);
            //if (nullptr != engine)
            //    engine->handleMouseMove(x, y);
        }
        break;
    case WM_CONTEXTMENU: {
            // TODO:
        }
        break;
    }

    return (DefWindowProc(hwnd, msg, wParam, lParam));
}

Window::Window(int width, int height, Type type)
{
    if (0 == g_windowCount) {
        SetProcessDPIAware(); // Avoid window scaling
        
        WNDCLASSEX windowClass;
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_OWNDC;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = GetModuleHandle(NULL);
        windowClass.hIcon = NULL;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.hbrBackground = 0;
        windowClass.lpszMenuName = NULL;
        windowClass.lpszClassName = L"dust3dGlesWindow";
        windowClass.hIconSm = NULL;
        windowClass.lpfnWndProc = windowMessageHandler;
        RegisterClassEx(&windowClass);
    }

    RECT rect = {0, 0, width, height};
    int style = 0;
    switch (type) {
    case Type::Main:
        style = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_OVERLAPPEDWINDOW; // | WS_MAXIMIZE;
        break;
    case Type::Popup:
        style = WS_POPUP;
        break;
    }
    
    AdjustWindowRect(&rect, style, FALSE);

    m_internal.handle = CreateWindowEx(0,
        L"dust3dGlesWindow", 
        L"", 
        style, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        rect.right - rect.left, 
        rect.bottom - rect.top, 
        NULL, 
        NULL, 
        GetModuleHandle(NULL), 
        this);
    m_internal.display = GetDC(m_internal.handle);
}

void Window::setTitle(const std::string &string)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    std::u16string utf16String = utf16conv.from_bytes(string);
    
    SetWindowTextW(m_internal.handle, reinterpret_cast<LPCWSTR>(utf16String.c_str()));
}

void Window::setVisible(bool visible)
{
    ShowWindow(m_internal.handle, visible ? SW_SHOW : SW_HIDE);
}

void Window::setEngine(IndieGameEngine *engine)
{
    m_engine = engine;
}

IndieGameEngine *Window::engine() const
{
    return m_engine;
}

WindowInternal &Window::internal()
{
    return m_internal;
}

static void windowTimerHandler(HWND hwnd, UINT msg, UINT_PTR timerId, DWORD time)
{
    Window *window = (Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto &timer = window->internal().timers[(timerId - 1)];
    timer();
}

void Window::addTimer(uint32_t milliseconds, std::function<void (void)> handler)
{
    m_internal.timers.push_back(handler);
    SetTimer(m_internal.handle, (UINT_PTR)m_internal.timers.size(), (UINT)milliseconds, windowTimerHandler);
}

void Window::mainLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Window::isKeyPressed(char key)
{
    return GetKeyState(key) & 0x8000;
}

uint64_t Window::getMilliseconds()
{
    static uint64_t numPerMilliseconds = 0;
    if (0 == numPerMilliseconds) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        numPerMilliseconds = frequency.QuadPart / 1000;
    }
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return (uint64_t)ticks.QuadPart / numPerMilliseconds;
}

}
