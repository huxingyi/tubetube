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
#include <Windows.h>
#include <hu/gles/indie_game_engine.h>
#include <hu/gles/window.h>

namespace Hu
{

static int g_windowCount = 0;

static void handlePendingMessage(Window *window, unsigned int msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_SIZE: {
            auto windowWidth = LOWORD(lparam);
            auto windowHeight = HIWORD(lparam);
            if (nullptr != window && nullptr != window->engine())
                window->engine()->setWindowSize(windowWidth, windowHeight);
        }
        break;
    case WM_MOUSEMOVE: {
            auto x = GET_X_LPARAM(lparam);
            auto y = GET_Y_LPARAM(lparam);
            if (nullptr != window && nullptr != window->engine())
                window->engine()->handleMouseMove(x, y);
        }
        break;
    case WM_CONTEXTMENU: {
            if (nullptr != window && nullptr != window->engine())
                window->engine()->shouldPopupMenu.emit();
        }
        break;
    case WM_LBUTTONDOWN: {
            if (nullptr != window && nullptr != window->engine())
                window->engine()->handleMouseLeftButtonDown();
        }
        break;
    case WM_LBUTTONUP: {
            if (nullptr != window && nullptr != window->engine())
                window->engine()->handleMouseLeftButtonUp();
        }
        break;
    }
}

static LRESULT CALLBACK windowMessageHandler(HWND hwnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {
    Window *window = (Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams); 
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
            if (window->closed.isEmpty())
                DestroyWindow(hwnd);
            else
                window->closed.emit();
            return 0;
        } 
        break;
    case WM_SIZE: {
            window->internal().pendingMessages.push({msg, wparam, lparam});
            //auto windowWidth = LOWORD(lparam);
            //auto windowHeight = HIWORD(lparam);
            //if (nullptr != window && nullptr != window->engine())
            //    window->engine()->setWindowSize(windowWidth, windowHeight);
            return 0;
        } 
        break;
    case WM_MOUSEWHEEL: {
            //auto zDelta = GET_WHEEL_DELTA_WPARAM(wparam);
            //fov += zDelta > 0.0 ? -5.0 : 5.0;
            //if (fov < 1.0f)
            //    fov = 1.0f;
            //if (fov > 45.0f)
            //    fov = 45.0f; 
        } 
        break;
    case WM_MOUSEMOVE: {
            window->internal().pendingMessages.push({msg, wparam, lparam});
            //auto x = GET_X_LPARAM(lparam);
            //auto y = GET_Y_LPARAM(lparam);
            //if (nullptr != window && nullptr != window->engine())
            //    window->engine()->handleMouseMove(x, y);
        }
        break;
    case WM_CONTEXTMENU: {
            window->internal().pendingMessages.push({msg, wparam, lparam});
            //if (nullptr != window && nullptr != window->engine())
            //    window->engine()->shouldPopupMenu.emit();
        }
        break;
    case WM_LBUTTONDOWN: {
            window->internal().pendingMessages.push({msg, wparam, lparam});
            //Window *popup = window->popupWindow();
            //if (nullptr != popup)
            //    popup->setVisible(false);
            //if (nullptr != window && nullptr != window->engine())
            //    window->engine()->handleMouseLeftButtonDown();
        }
        break;
    case WM_LBUTTONUP: {
            window->internal().pendingMessages.push({msg, wparam, lparam});
            //if (nullptr != window && nullptr != window->engine())
            //    window->engine()->handleMouseLeftButtonUp();
        }
        break;
        /*
    case WM_ACTIVATE: {
            if (WA_INACTIVE == LOWORD(wparam)) {
                if (Window::Type::Popup == window->type()) {
                    window->setVisible(true);
                } else {
                    Window *popup = window->popupWindow();
                    if (nullptr != popup)
                        popup->setVisible(false);
                }
            }
        }
        break;
    case WM_SETFOCUS: {
            if (Window::Type::Popup == window->type()) {
                Window *parent = window->parentWindow();
                if (nullptr != parent)
                    SetFocus(parent->internal().handle);
            }
        }
        break;
        */
    }

    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

static EGLint getContextRenderableType(EGLDisplay eglDisplay)
{
#ifdef EGL_KHR_create_context
    const char *extensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    if (extensions != NULL && strstr(extensions, "EGL_KHR_create_context")) {
        return EGL_OPENGL_ES3_BIT_KHR;
    }
#endif
   return EGL_OPENGL_ES2_BIT;
}

Window::~Window()
{
    setVisible(false);
    SetWindowLongPtr(m_internal.handle, GWLP_USERDATA, (LONG_PTR)0);
    
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   
    delete m_engine;
    m_engine = nullptr;
    
    eglDestroyContext(m_eglDisplay, m_eglContext);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    
    eglTerminate(m_eglDisplay);
    
    DestroyWindow(m_internal.handle);
}

Window::Window(int width, int height, Type type, Window *parent):
    m_type(type),
    m_parentWindow(parent)
{
    if (0 == g_windowCount) {
        SetProcessDPIAware(); // Avoid window scaling
        
        WNDCLASSEX windowClass;
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_OWNDC;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = GetModuleHandle(NULL);
        windowClass.hIcon = LoadIcon(GetModuleHandle(NULL), L"IDI_ICON1");
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
    int styleEx = 0;
    switch (m_type) {
    case Type::Main:
        style = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_OVERLAPPEDWINDOW; // | WS_MAXIMIZE;
        break;
    case Type::Popup:
        style = WS_POPUP;
        break;
    }
    
    AdjustWindowRect(&rect, style, FALSE);

    m_internal.handle = CreateWindowEx(styleEx,
        L"dust3dGlesWindow", 
        L"", 
        style, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        rect.right - rect.left, 
        rect.bottom - rect.top, 
        nullptr == m_parentWindow ? NULL : m_parentWindow->internal().handle, 
        NULL, 
        GetModuleHandle(NULL), 
        this);
    m_internal.display = GetDC(m_internal.handle);
    
    m_eglDisplay = eglGetDisplay(m_internal.display);
    if (EGL_NO_DISPLAY == m_eglDisplay)
        huDebug << "eglGetDisplay returns EGL_NO_DISPLAY";
    
    EGLint eglVersionMajor, eglVersionMinor;
    eglInitialize(m_eglDisplay, &eglVersionMajor, &eglVersionMinor);
    
    EGLint configAttributes[] = {
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         24,
        EGL_STENCIL_SIZE,       8,
        EGL_SAMPLE_BUFFERS,     1,
        EGL_SAMPLES,            4,
        EGL_RENDERABLE_TYPE,    getContextRenderableType(m_eglDisplay),
        EGL_NONE
    };

    EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 
        3, 
        EGL_NONE
    };
    
    EGLint numConfigs;
    EGLConfig windowConfig;
    eglChooseConfig(m_eglDisplay, configAttributes, &windowConfig, 1, &numConfigs);
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, windowConfig, m_internal.handle, NULL);
    if (EGL_NO_SURFACE == m_eglSurface)
        huDebug << "eglCreateWindowSurface returns EGL_NO_SURFACE:" << eglGetError();
    
    m_eglContext = eglCreateContext(m_eglDisplay, windowConfig, EGL_NO_CONTEXT, contextAttributes);
    if (EGL_NO_CONTEXT == m_eglContext)
        huDebug << "eglCreateContext returns EGL_NO_SURFACE:" << eglGetError();
    
    setEngine(new IndieGameEngine);
    m_engine->setWindow(this);
    m_engine->setMillisecondsQueryHandler([]() {
        return Window::getMilliseconds();
    });
    m_engine->setWindowSize(static_cast<double>(width), static_cast<double>(height));
    
    addTimer(1000 / 300, [=]() {
        this->engine()->update();
    });
    addTimer(1000 / 60, [=]() {
        eglMakeCurrent(this->eglDisplay(), this->eglSurface(), this->eglSurface(), this->eglContext());
        if (!this->m_internal.pendingMessages.empty())
            this->engine()->dirty();
        this->engine()->renderScene();
        eglSwapBuffers(this->eglDisplay(), this->eglSurface());
        while (!this->m_selectSingleFileRequests.empty()) {
            auto filterSurfixList = this->m_selectSingleFileRequests.front();
            this->m_selectSingleFileRequests.pop();
            this->m_selectSingleFileResults.push(this->selectSingleFileByUser(filterSurfixList));
        }
    });
}

void Window::update()
{
    while (!m_internal.pendingMessages.empty()) {
        auto pending = m_internal.pendingMessages.front();
        m_internal.pendingMessages.pop();
        handlePendingMessage(this, pending.msg, pending.wparam, pending.lparam);
    }
    while (!m_selectSingleFileResults.empty()) {
        auto result = m_selectSingleFileResults.front();
        m_selectSingleFileResults.pop();
        fileSelected.emit(result);
    }
}

void Window::setTitle(const std::string &string)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    std::u16string utf16String = utf16conv.from_bytes(string);
    
    SetWindowTextW(m_internal.handle, reinterpret_cast<LPCWSTR>(utf16String.c_str()));
}

void Window::setVisible(bool visible)
{
    if (visible) {
        WINDOWPLACEMENT place;
        memset(&place, 0, sizeof(WINDOWPLACEMENT));
        place.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(m_internal.handle, &place);

        switch (place.showCmd) {
        case SW_SHOWMAXIMIZED:
            ShowWindow(m_internal.handle, SW_SHOWMAXIMIZED);
            break;
        case SW_SHOWMINIMIZED:
            ShowWindow(m_internal.handle, SW_RESTORE);
            break;
        default:
            ShowWindow(m_internal.handle, SW_NORMAL);
            break;
        }
        return;
    }
    ShowWindow(m_internal.handle, SW_HIDE);
}

void Window::setEngine(IndieGameEngine *engine)
{
    m_engine = engine;
}

EGLDisplay Window::eglDisplay() const
{
    return m_eglDisplay;
}

EGLDisplay Window::eglSurface() const
{
    return m_eglSurface;
}

EGLDisplay Window::eglContext() const
{
    return m_eglContext;
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

std::pair<int, int> Window::getCursorPosition()
{
    POINT point;
    GetCursorPos(&point);
    return {(int)point.x, (int)point.y};
}

void Window::setPosition(int x, int y)
{
    SetWindowPos(m_internal.handle, NULL, x, y, 0, 0, SWP_NOSIZE);
}

Window::Type Window::type() const
{
    return m_type;
}

Window *Window::openPopupWindow()
{
    if (nullptr == m_popupWindow)
        m_popupWindow = new Window(100, 50, Window::Type::Popup, this);
    auto position = Window::getCursorPosition();
    m_popupWindow->setPosition(position.first, position.second);
    return m_popupWindow;
}

Window *Window::popupWindow() const
{
    return m_popupWindow;
}

Window *Window::parentWindow() const
{
    return m_parentWindow;
}

void Window::requestToSelectSingleFile(const std::vector<std::string> &filterSurfixList)
{
    m_selectSingleFileRequests.push(filterSurfixList);
}

std::string Window::selectSingleFileByUser(const std::vector<std::string> &filterSurfixList) const
{
    OPENFILENAMEW ofn;
    wchar_t path[260] = {0};
    
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
    
    std::wstring filterString;
    if (filterSurfixList.empty()) {
        filterString = L"All\0*.*\0\0";
    } else {
        for (const auto &surfix: filterSurfixList) {
            if (!filterString.empty())
                filterString += L";";
            auto wideSurfix = utf16conv.from_bytes(surfix);
            filterString += L"*.";
            filterString += reinterpret_cast<LPCWSTR>(wideSurfix.c_str());
        }
        filterString = filterString + L"|" + filterString + L"|";
    }
    wchar_t *filterChars = (wchar_t *)filterString.c_str();
    for (wchar_t *p = filterChars; *p; ++p) {
        if (L'|' == *p)
            *p = L'\0';
    }

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_internal.handle;
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.lpstrFilter = filterChars;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (!GetOpenFileNameW(&ofn))
        return "";
    
    std::u16string utf16Path(std::begin(path), std::begin(path) + wcslen(path));
    return std::move(utf16conv.to_bytes(utf16Path));
}

void Window::bringToForeground()
{
    SetActiveWindow(m_internal.handle);
}

void Window::mainLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

}
