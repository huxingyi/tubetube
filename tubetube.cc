#include <dust3d/base/matrix4x4.h>
#include <dust3d/gles/vertex_buffer.h>
#include <dust3d/gles/vertex_buffer_utils.h>
#include <dust3d/gles/indie_game_engine.h>
#include <dust3d/mesh/tube_mesh_builder.h>
#include <Windows.h>
#include <Windowsx.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>

static int windowWidth = 640;
static int windowHeight = 360;
static HWND windowHandle = nullptr;
static bool quit = false;

using namespace dust3d;

static LRESULT CALLBACK wndProc(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE : {
                quit = true;
                DestroyWindow(hwnd);
                PostQuitMessage(0);
                return 0;
            } break;
        case WM_SIZE: {
                windowWidth = LOWORD(lParam);
                windowHeight = HIWORD(lParam);
                return 0;
            } break;
        case WM_MOUSEWHEEL: {
                //auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                //fov += zDelta > 0.0 ? -5.0 : 5.0;
                //if (fov < 1.0f)
                //    fov = 1.0f;
                //if (fov > 45.0f)
                //    fov = 45.0f; 
            } break;
        case WM_MOUSEMOVE: {
                auto x = GET_X_LPARAM(lParam); 
                auto y = GET_Y_LPARAM(lParam);
                IndieGameEngine::indie()->handleMouseMove(x, y);
            } break;
    }

    return (DefWindowProc(hwnd, msg, wParam, lParam));
}

static HWND createWindow(int width, int height) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wcex;
    
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = &DefWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = 0;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "eglsamplewnd";
    wcex.hIconSm = NULL;
    wcex.lpfnWndProc = wndProc;

    RegisterClassEx(&wcex);
    RECT rect = { 0, 0, width, height };
    int style = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
    AdjustWindowRect(&rect, style, FALSE);

    HWND hwnd = CreateWindow("eglsamplewnd", 
        "Tubetube", 
        style, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        rect.right - rect.left, 
        rect.bottom - rect.top, 
        NULL, 
        NULL, 
        GetModuleHandle(NULL), 
        NULL);
    ShowWindow(hwnd, SW_SHOW);

    return hwnd;
}

std::vector<Vector3> modelPositions = {
    Vector3( 0.0f, 0.0f, 0.0f),
    Vector3( 2.0f, 5.0f, -15.0f),
    Vector3(-1.5f, -2.2f, -2.5f),
    Vector3(-3.8f, -2.0f, -12.3f),
    Vector3( 2.4f, -0.4f, -3.5f),
    Vector3(-1.7f, 3.0f, -7.5f),
    Vector3( 1.3f, -2.0f, -2.5f),
    Vector3( 1.5f, 2.0f, -2.5f),
    Vector3( 1.5f, 0.2f, -1.5f),
    Vector3(-1.3f, 1.0f, -1.5f)
};

static EGLint getContextRenderableType ( EGLDisplay eglDisplay )
{
#ifdef EGL_KHR_create_context
    const char *extensions = eglQueryString (eglDisplay, EGL_EXTENSIONS);
    if (extensions != NULL && strstr(extensions, "EGL_KHR_create_context")) {
        return EGL_OPENGL_ES3_BIT_KHR;
    }
#endif
   return EGL_OPENGL_ES2_BIT;
}

static std::unique_ptr<std::vector<VertexBuffer>> loadResouceVertexBufferList(const std::string &resourceName)
{
    if ("Ground" == resourceName) {
        VertexBuffer vertexBuffer(std::make_unique<std::vector<GLfloat>>(std::vector<GLfloat> {
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-left
             0.5f,  0.5f , 0.5f,  0.0f,  1.0f,  0.0f, // bottom-right
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-right     
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // bottom-right
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-left
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f  // bottom-left
        }), 6, 6, IndieGameEngine::DrawHint::Triangles);
        auto vertexBufferList = std::make_unique<std::vector<VertexBuffer>>();
        vertexBufferList->push_back(std::move(vertexBuffer));
        return std::move(vertexBufferList);
    } else if ("Plane" == resourceName) {
        auto sections = std::make_unique<std::vector<TubeMeshBuilder::Section>>(std::vector<TubeMeshBuilder::Section> {
            #include <dust3d/samples/make_plane/plane.sections>
        });
        TubeMeshBuilder tubeMeshBuilder(std::move(sections));
        tubeMeshBuilder.build();
        auto meshVertices = tubeMeshBuilder.takeMeshVertices();
        std::vector<std::vector<size_t>> meshTriangles;
        tubeMeshBuilder.getMeshTriangles(meshTriangles);
        auto meshQuads = tubeMeshBuilder.takeMeshQuads();
        auto meshProfileEdges = tubeMeshBuilder.takeMeshProfileEdges();
        VertexBuffer modelBuffer;
        VertexBuffer modelBorderBuffer;
        VertexBufferUtils::loadTrangulatedMesh(modelBuffer, *meshVertices, meshTriangles, IndieGameEngine::DrawHint::Triangles);
        VertexBufferUtils::loadMeshProfileEdges(modelBorderBuffer, *meshVertices, *meshProfileEdges, IndieGameEngine::DrawHint::Lines);
        auto vertexBufferList = std::make_unique<std::vector<VertexBuffer>>();
        vertexBufferList->push_back(std::move(modelBuffer));
        vertexBufferList->push_back(std::move(modelBorderBuffer));
        return std::move(vertexBufferList);
    }
    return nullptr;
}

static bool queryKeyPressed(char key)
{
    return GetKeyState(key) & 0x8000;
}

int main(int argc, char* argv[])
{
    windowHandle = createWindow(windowWidth, windowHeight);
    HDC hdc = GetDC(windowHandle);

    EGLDisplay eglDisplay = eglGetDisplay(hdc);
    if (eglDisplay == EGL_NO_DISPLAY) {
        std::cout << "Could not get egl display!" << std::endl;
        return 1;
    }

    EGLint eglVersionMajor, eglVersionMinor;
    eglInitialize(eglDisplay, &eglVersionMajor, &eglVersionMinor);
    
    EGLint configAttributes[] = {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_STENCIL_SIZE,   8,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES,        4,
        EGL_RENDERABLE_TYPE, getContextRenderableType(eglDisplay),
        EGL_NONE
    };

    EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    
    EGLint nrOfConfigs;
    EGLConfig windowConfig;
    eglChooseConfig(eglDisplay, configAttributes, &windowConfig, 1, &nrOfConfigs);
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, windowConfig, windowHandle, NULL);
    if (eglSurface == EGL_NO_SURFACE) {
        std::cerr << "Could not create EGL surface : " << eglGetError() << std::endl;
        return 1;
    }

    EGLContext eglContext = eglCreateContext(eglDisplay, windowConfig, EGL_NO_CONTEXT, contextAttributes);

    if (eglContext == EGL_NO_CONTEXT) {
        std::cout << "Could not create egl context : " << eglGetError() << std::endl;
        return 1;
    }
    
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    
    IndieGameEngine::indie()->setWindowSize(static_cast<double>(windowWidth), static_cast<double>(windowHeight));
    IndieGameEngine::indie()->setVertexBufferListLoadHandler(loadResouceVertexBufferList);
    IndieGameEngine::indie()->setKeyPressedQueryHandler(queryKeyPressed);
    {
        Matrix4x4 modelMatrix;
        modelMatrix.translate(Vector3(0.0, -3.1, 0.0));
        modelMatrix.scale(Vector3(20.0, 0.0, 20.0));
        IndieGameEngine::indie()->addObject("defaultGround", "Ground", modelMatrix, IndieGameEngine::RenderType::Ground);
    }
    for (size_t i = 0; i < modelPositions.size(); ++i) {
        Matrix4x4 modelMatrix;
        modelMatrix.translate(modelPositions[i]);
        IndieGameEngine::indie()->addObject("plane" + ('1' + i), "Plane", modelMatrix, IndieGameEngine::RenderType::Default);
    }

    MSG uMsg;
    PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE);

    while (!quit)  {
        IndieGameEngine::indie()->update();
        IndieGameEngine::indie()->renderScene();
        while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&uMsg);
            DispatchMessage(&uMsg);
        }
        eglSwapBuffers(eglDisplay, eglSurface);
    }
    
    return 0;
}