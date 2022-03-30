#define NOMINMAX
#include <dust3d/base/matrix4x4.h>
#include <dust3d/gles/vertex_buffer.h>
#include <dust3d/gles/vertex_buffer_utils.h>
#include <dust3d/gles/indie_game_engine.h>
#include <dust3d/gles/terrain_generator.h>
#include <dust3d/mesh/tube_mesh_builder.h>
#include <dust3d/mesh/mesh_utils.h>
#include <Windows.h>
#include <Windowsx.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <random>

static int windowWidth = 640;
static int windowHeight = 360;
static HWND windowHandle = nullptr;
static bool quit = false;
EGLDisplay eglDisplay = EGL_NO_DISPLAY;
EGLSurface eglSurface = EGL_NO_SURFACE;
//std::random_device randomSeeder;
std::mt19937 randomEngine(0); //std::mt19937 randomEngine(randomSeeder());
std::uniform_real_distribution<double> randomSpawn(-3.0, 3.0);
std::uniform_real_distribution<double> randomReal(0.0, 1.0);
auto spawn = std::bind(randomSpawn, randomEngine);
auto rand01 = std::bind(randomReal, randomEngine);

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
                //IndieGameEngine::indie()->handleMouseMove(x, y);
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

static std::unique_ptr<std::vector<VertexBuffer>> buildVertexBufferListFromSections(std::unique_ptr<std::vector<TubeMeshBuilder::Section>> sections)
{
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

static std::unique_ptr<std::vector<VertexBuffer>> loadResouceVertexBufferList(const std::string &resourceName)
{
    if ("Sea" == resourceName) {
        GLfloat waterColor[] = {0.58f, 0.62f, 0.55f};
        VertexBuffer vertexBuffer(std::make_unique<std::vector<GLfloat>>(std::vector<GLfloat> {
            -0.5f,  0.0f, -0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2], // top-left
             0.5f,  0.0f , 0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2], // bottom-right
             0.5f,  0.0f, -0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2], // top-right     
             0.5f,  0.0f,  0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2], // bottom-right
            -0.5f,  0.0f, -0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2], // top-left
            -0.5f,  0.0f,  0.5f,  0.0f,  1.0f,  0.0f, waterColor[0], waterColor[1], waterColor[2]  // bottom-left
        }), 9, 6, IndieGameEngine::DrawHint::Triangles);
        auto vertexBufferList = std::make_unique<std::vector<VertexBuffer>>();
        vertexBufferList->push_back(std::move(vertexBuffer));
        return std::move(vertexBufferList);
    } else if ("Ground" == resourceName) {
        TerrainGenerator terrainGenerator;
        double frequency = 5.0;
        terrainGenerator.generate(frequency);
        std::vector<Vector3> vertices;
        std::vector<std::vector<size_t>> triangles;
        terrainGenerator.getTriangulatedMesh(vertices, triangles, 1.0, 20.0);
        double heightOffset = -0.5;
        double heightScale = 2.0;
        VertexBuffer vertexBuffer;
        auto vertexBufferVertices = std::make_unique<std::vector<GLfloat>>();
        size_t numbersPerVertex = 9;
        vertexBufferVertices->resize(triangles.size() * 3 * numbersPerVertex);
        std::vector<Vector3> colors(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            colors[i] = TerrainGenerator::heightToColor(vertices[i].y());
            vertices[i].y() = vertices[i].y() * heightScale + heightOffset;
        }
        std::vector<Vector3> triangleNormals(triangles.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            const auto &triangle = triangles[i];
            triangleNormals[i] = Vector3::normal(vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]]);
        }
        std::vector<Vector3> triangleVertexNormals;
        MeshUtils::smoothNormal(vertices, triangles, triangleNormals, Math::radiansFromDegrees(90.0), triangleVertexNormals);
        for (size_t i = 0, targetIndex = 0; i < triangles.size(); ++i) {
            const auto &triangle = triangles[i];
            size_t normalIndex = i * 3;
            const auto &firstNormal = triangleVertexNormals[normalIndex + 0];
            const auto &secondNormal = triangleVertexNormals[normalIndex + 1];
            const auto &thirdNormal = triangleVertexNormals[normalIndex + 2];
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)firstNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)firstNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)firstNormal.z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[0]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[0]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[0]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)secondNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)secondNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)secondNormal.z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[1]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[1]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[1]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)thirdNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)thirdNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)thirdNormal.z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[2]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[2]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)colors[triangle[2]].z();
        }
        size_t vertexCount = vertexBufferVertices->size() / numbersPerVertex;
        vertexBuffer.update(std::move(vertexBufferVertices), numbersPerVertex, vertexCount, IndieGameEngine::DrawHint::Triangles);
        auto vertexBufferList = std::make_unique<std::vector<VertexBuffer>>();
        VertexBuffer edgeVertexBuffer;
        VertexBufferUtils::loadMeshBorders(edgeVertexBuffer, vertices, triangles, IndieGameEngine::DrawHint::Lines);
        vertexBufferList->push_back(std::move(vertexBuffer));
        vertexBufferList->push_back(std::move(edgeVertexBuffer));
        return std::move(vertexBufferList);
    } else if ("Plane" == resourceName) {
        auto sections = std::make_unique<std::vector<TubeMeshBuilder::Section>>(std::vector<TubeMeshBuilder::Section> {
            #include <dust3d/samples/make_plane/plane.sections>
        });
        return buildVertexBufferListFromSections(std::move(sections));
    } else if ("Artillery" == resourceName) {
        auto sections = std::make_unique<std::vector<TubeMeshBuilder::Section>>(std::vector<TubeMeshBuilder::Section> {
            #include <dust3d/samples/make_artillery/artillery.sections>
        });
        return buildVertexBufferListFromSections(std::move(sections));
    }
    return nullptr;
}

static bool queryKeyPressed(char key)
{
    return GetKeyState(key) & 0x8000;
}

static void updateTimer(HWND hwnd, UINT msg, UINT_PTR timerId, DWORD time)
{
    IndieGameEngine::indie()->update();
}

static void renderTimer(HWND hwnd, UINT msg, UINT_PTR timerId, DWORD time)
{
    IndieGameEngine::indie()->renderScene();
    eglSwapBuffers(eglDisplay, eglSurface);
}

class DummyPlaneLocationState: public IndieGameEngine::LocationState
{
public:
    bool update()
    {
        if (Math::isZero(speed))
            return false;
        
        if (m_forwardAcceleration > 0)
            m_forwardAcceleration -= 0.01;
        
        if (m_forwardAcceleration > 0)
            speed += m_forwardAcceleration * IndieGameEngine::indie()->elapsedSecondsSinceLastUpdate();

        worldLocation += velocity() * IndieGameEngine::indie()->elapsedSecondsSinceLastUpdate();
        const double tailFlameRadius = 0.03;
        double tailFlameSpeed = speed * 0.1;
        uint64_t emitInterval = (tailFlameRadius * 0.5 / speed) * 1000;
        if (m_lastEmitTime + emitInterval < IndieGameEngine::indie()->millisecondsSinceStart()) {
            IndieGameEngine::indie()->addParticle(1.5, tailFlameRadius, worldLocation - forwardDirection * 0.2 + Vector3(0.0, 0.02, 0.0), forwardDirection * tailFlameSpeed, Vector3(0.95, 0.9, 0.27), Vector3(0.58, 0.31, 0.22));
            m_lastEmitTime = IndieGameEngine::indie()->millisecondsSinceStart();
        }
        return true;
    }

private:
    uint64_t m_lastEmitTime = 0;
    double m_forwardAcceleration = rand01();
};

class PlayerLocationState: public IndieGameEngine::LocationState
{
public:
    bool update()
    {
        if (!Math::isZero(m_forwardAcceleration))
            speed += m_forwardAcceleration * IndieGameEngine::indie()->elapsedSecondsSinceLastUpdate();

        worldLocation += velocity() * IndieGameEngine::indie()->elapsedSecondsSinceLastUpdate();
        const double tailFlameRadius = 0.03;
        double tailFlameSpeed = speed * 0.5;
        uint64_t emitInterval = (tailFlameRadius * 0.5 / speed) * 1000;
        if (m_lastEmitTime + emitInterval < IndieGameEngine::indie()->millisecondsSinceStart()) {
            IndieGameEngine::indie()->addParticle(1.5, tailFlameRadius, worldLocation - forwardDirection * 0.2 + Vector3(0.0, 0.02, 0.0), forwardDirection * tailFlameSpeed, Vector3(0.95, 0.9, 0.27), Vector3(0.58, 0.31, 0.22));
            m_lastEmitTime = IndieGameEngine::indie()->millisecondsSinceStart();
        }
        
        const double responseSpeed = 1.0 * IndieGameEngine::indie()->elapsedSecondsSinceLastUpdate();
        
        if (queryKeyPressed(' ') || queryKeyPressed('W')) {
            m_forwardAcceleration = std::min(m_forwardAcceleration + 1.0 * responseSpeed, m_maxForwardAcceleration);
        } else if (queryKeyPressed('S')) {
            m_forwardAcceleration = std::max(m_forwardAcceleration - 1.0 * responseSpeed, -m_maxForwardAcceleration);
        } else {
            m_forwardAcceleration = 0.0;
        }
        
        if (queryKeyPressed('R')) {
            Vector3 xDirection = Vector3::crossProduct(forwardDirection, upDirection);
            forwardDirection = forwardDirection.rotated(xDirection, responseSpeed * Math::radiansFromDegrees(10.0));
            upDirection = Vector3::crossProduct(xDirection, forwardDirection);
        } else if (queryKeyPressed('F')) {
            Vector3 xDirection = Vector3::crossProduct(forwardDirection, upDirection);
            forwardDirection = forwardDirection.rotated(xDirection, responseSpeed * Math::radiansFromDegrees(-10.0));
            upDirection = Vector3::crossProduct(xDirection, forwardDirection);
        }

        if (queryKeyPressed('A')) {
            upDirection = upDirection.rotated(forwardDirection, responseSpeed * Math::radiansFromDegrees(-10.0));
            forwardDirection = forwardDirection.rotated(upDirection, responseSpeed * Math::radiansFromDegrees(-5.0));
            return true;
        } else if (queryKeyPressed('D')) {
            upDirection = upDirection.rotated(forwardDirection, responseSpeed * Math::radiansFromDegrees(10.0));
            forwardDirection = forwardDirection.rotated(upDirection, responseSpeed * Math::radiansFromDegrees(5.0));
            return true;
        }
        
        if (!Math::isZero(speed))
            return true;
        
        return false;
    }
    
private:
    uint64_t m_lastEmitTime = 0;
    double m_forwardAcceleration = 0.0;
    const double m_maxForwardAcceleration = 1.0;
};

class WorldState: public IndieGameEngine::State
{
public:
    bool update()
    {
        if (IndieGameEngine::indie()->objectCount() < 50) {
            auto objectId = "plane" + std::to_string(IndieGameEngine::indie()->objectCount());
            IndieGameEngine::indie()->addObject(objectId, "Plane", Matrix4x4(), IndieGameEngine::RenderType::Default);
            auto dummyPlaneState = std::make_unique<DummyPlaneLocationState>();
            dummyPlaneState->worldLocation = Vector3(spawn(), spawn() + 3.0, spawn());
            dummyPlaneState->forwardDirection = Vector3(0.0, 0.0, -1.0);
            dummyPlaneState->speed = spawn() * 0.15;
            IndieGameEngine::indie()->addLocationState(objectId, std::move(dummyPlaneState));
            return true;
        }
        return false;
    }
};

int main(int argc, char* argv[])
{
    windowHandle = createWindow(windowWidth, windowHeight);
    HDC hdc = GetDC(windowHandle);

    eglDisplay = eglGetDisplay(hdc);
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
        EGL_SAMPLE_BUFFERS, 0,  // Multisamples is enabled in framebuffer see dust3d/gles/screen_map.h
        EGL_RENDERABLE_TYPE, getContextRenderableType(eglDisplay),
        EGL_NONE
    };

    EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    
    EGLint nrOfConfigs;
    EGLConfig windowConfig;
    eglChooseConfig(eglDisplay, configAttributes, &windowConfig, 1, &nrOfConfigs);
    eglSurface = eglCreateWindowSurface(eglDisplay, windowConfig, windowHandle, NULL);
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
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    uint64_t numPerMilliseconds = frequency.QuadPart / 1000;
    IndieGameEngine::indie()->setMillisecondsQueryHandler([=]() {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);
        return (uint64_t)ticks.QuadPart / numPerMilliseconds;
    });
    IndieGameEngine::indie()->setWindowSize(static_cast<double>(windowWidth), static_cast<double>(windowHeight));
    IndieGameEngine::indie()->setVertexBufferListLoadHandler(loadResouceVertexBufferList);
    //IndieGameEngine::indie()->setKeyPressedQueryHandler(queryKeyPressed);
    {
        Matrix4x4 modelMatrix;
        IndieGameEngine::indie()->addObject("defaultGround", "Ground", modelMatrix, IndieGameEngine::RenderType::Ground);
    }
    {
        Matrix4x4 modelMatrix;
        modelMatrix.translate(Vector3(0.0, -0.001, 0.0));
        modelMatrix.scale(Vector3(1000.0, 0.0, 1000.0));
        IndieGameEngine::indie()->addObject("defaultSea", "Sea", modelMatrix, IndieGameEngine::RenderType::Water);
    }
    {
        IndieGameEngine::indie()->addObject("palyer0", "Plane", Matrix4x4(), IndieGameEngine::RenderType::Default);
        auto playerState = std::make_unique<PlayerLocationState>();
        playerState->worldLocation = IndieGameEngine::indie()->cameraPosition();
        playerState->forwardDirection = (IndieGameEngine::indie()->cameraTarget() - IndieGameEngine::indie()->cameraPosition()).normalized();
        playerState->upDirection = IndieGameEngine::indie()->cameraUp();
        playerState->followedByCamera = true;
        IndieGameEngine::indie()->addLocationState("palyer0", std::move(playerState));
    }
    IndieGameEngine::indie()->addGeneralState("", std::make_unique<WorldState>());

    SetTimer(windowHandle, 1, 1000 / 300, updateTimer);
    SetTimer(windowHandle, 2, 1000 / 60, renderTimer);
    while (!quit)  {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return 0;
}