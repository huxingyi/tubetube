#include <dust3d/base/matrix4x4.h>
#include <dust3d/gles/shader.h>
#include <dust3d/gles/vertex_buffer.h>
#include <dust3d/gles/vertex_buffer_utils.h>
#include <dust3d/gles/shadow_map.h>
#include <dust3d/mesh/tube_mesh_builder.h>
#include <Windows.h>
#include <Windowsx.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <sstream>
#include <iostream>
#include <array>
#include <random>
#include "third_party/tga_utils/tga_utils.h"
#define _USE_MATH_DEFINES
#include <math.h>

static int windowWidth = 640;
static int windowHeight = 360;
static bool quit = false;

using namespace dust3d;

LONGLONG qpcFrequency;
double timeFactor = 1.0f;

void glErrorLog() {
	GLuint glError = glGetError();
	if (glError != GL_NO_ERROR) {
		std::cout << "OpenGL Error : " << glError << "\n";
	}
}

std::unique_ptr<std::vector<GLfloat>> groundVertices = std::unique_ptr<std::vector<GLfloat>>(new std::vector<GLfloat> {
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-left
     0.5f,  0.5f , 0.5f,  0.0f,  1.0f,  0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-right     
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // top-left
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f  // bottom-left
});

std::unique_ptr<std::vector<GLfloat>> quadVertices = std::unique_ptr<std::vector<GLfloat>>(new std::vector<GLfloat> {
    -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
});

HWND windowHandle = nullptr;
VertexBuffer modelBuffer;
VertexBuffer groundBuffer;
VertexBuffer quadBuffer;
VertexBuffer modelBorderBuffer;
Shader modelShader;
Shader lightShader;
Shader debugQuadShader;
Shader particleShader;
Shader singleColorShader;
ShadowMap shadowMap;
Vector3 modelRotation;
Vector3 lightPosition;
Vector3 cameraPosition;
Vector3 cameraFront;
Vector3 cameraUp;
GLuint smokeTextureId = 0;
struct Particle
{
    float lifetime;
    float startPosition[3];
    float endPosition[3];
};
static const size_t particleCount = 1024;
std::array<Particle, particleCount> particles;
std::random_device randomDevice;
std::mt19937 randomEngine(randomDevice());
std::uniform_real_distribution<float> randomBetween0to1(0.0f, 1.0f);
std::uniform_real_distribution<float> randomBetween0to2(0.0f, 2.0f);
std::uniform_real_distribution<float> randomBetween0to2Pi(0.0f, 2.0f * float(M_PI));
std::uniform_real_distribution<float> randomBetween0to2p5(0.0f, 2.5f);
std::uniform_real_distribution<float> randomBetweenM0p5to0p5(-0.5f, 0.5f);
float particleTime = 0.0;
static GLfloat rayCoords[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f
};
double fov = 45.0;
    
bool lightMoveIncrease = false;
std::pair<float, float> lightMoveEndpoints = {-3.0, 3.0};
float lightMoveStep = 0.05;

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
                static int s_lastX = 0;
                static int s_lastY = 0;
                static bool s_firstTime = true;
                if (s_firstTime) {
                    s_firstTime = false;
                    s_lastX = x;
                    s_lastY = y;
                }
                int offsetX = x - s_lastX;
                int offsetY = y - s_lastY;
                if (0 != offsetX) {
                    cameraFront = cameraFront.rotated(Vector3(0.0, 1.0, 0.0), Math::radiansFromDegrees(45.0 * -offsetX / windowWidth));
                }
                if (0 != offsetY) {
                    cameraFront = cameraFront.rotated(Vector3(1.0, 0.0, 0.0), Math::radiansFromDegrees(45.0 * -offsetY / windowHeight));
                }
                s_lastX = x;
                s_lastY = y;
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

void initScene() {
    QueryPerformanceCounter((LARGE_INTEGER*)&qpcFrequency);
	glClearColor(0.145f, 0.145f, 0.145f, 1.0f);
    
    {
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/phong.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/blinn-phong.frag>
            ;
        modelShader = Shader(vertexShaderSource, fragmentShaderSource);
    }
    {
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/phong.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/light.frag>
            ;
        lightShader = Shader(vertexShaderSource, fragmentShaderSource);
    }
    {
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/quad.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/debug-quad.frag>
            ;
        debugQuadShader = Shader(vertexShaderSource, fragmentShaderSource);
    }
    {
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/particle.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/particle.frag>
            ;
        particleShader = Shader(vertexShaderSource, fragmentShaderSource);
    }
    {
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/single-color.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/single-color.frag>
            ;
        singleColorShader = Shader(vertexShaderSource, fragmentShaderSource);
    }
    {
        auto sections = std::make_unique<std::vector<TubeMeshBuilder::Section>>(std::vector<TubeMeshBuilder::Section> {
            #include <dust3d/samples/make_plane/plane.sections>
        });
        TubeMeshBuilder tubeMeshBuilder(std::move(sections));
        tubeMeshBuilder.build();
        auto meshVertices = tubeMeshBuilder.takeMeshVertices();
        std::vector<std::vector<size_t>> meshTriangles;
        tubeMeshBuilder.getMeshTriangles(meshTriangles);
        auto meshQuads = tubeMeshBuilder.takeMeshQuads();
        VertexBufferUtils::loadTrangulatedMesh(modelBuffer, *meshVertices, meshTriangles);
        VertexBufferUtils::loadMeshBorders(modelBorderBuffer, *meshVertices, *meshQuads);
    }
    
    modelRotation = Vector3(15.0, 45.0, 0.0);
    lightPosition = Vector3(0.2, 1.0, 2.0);
    cameraPosition = Vector3(0.0, 0.0, 4.0);
    cameraFront = Vector3(0.0, 0.0, -1.0);
    cameraUp = Vector3(0.0, 1.0, 0.0);
    
    size_t groundVertexCount = groundVertices->size() / 6;
    groundBuffer.update(std::move(groundVertices), 6, groundVertexCount);
    
    size_t quadVertexCount = quadVertices->size() / 5;
    quadBuffer.update(std::move(quadVertices), 5, quadVertexCount);
    
    shadowMap.initialize();
    
    TGAImage img;
    LoadTGAImageFromFile("smoke.tga", &img);
    smokeTextureId = LoadTextureFromTGAImage(img);
    
    for (size_t i = 0; i < particleCount; i++) {
        particles[i].lifetime = randomBetween0to1(randomEngine);
        float endAngle = randomBetween0to2Pi(randomEngine);
        float endRadius = randomBetween0to2(randomEngine);
        particles[i].endPosition[0] = sinf(endAngle) * endRadius;
        particles[i].endPosition[1] = cosf(endAngle) * endRadius;
        particles[i].endPosition[2] = 0.0f;
        float startAngle = randomBetween0to2Pi(randomEngine);
        float startRadius = randomBetween0to2p5(randomEngine);
        particles[i].startPosition[0] = sinf(startAngle) * startRadius;
        particles[i].startPosition[1] = cosf(startAngle) * startRadius;
        particles[i].startPosition[2] = 0.0f;
    }
    particleTime = 1.0f;
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

void drawGround(Shader &shader)
{
    Matrix4x4 groundMatrix;
    groundMatrix.translate(Vector3(0.0, -3.1, 0.0));
    groundMatrix.scale(Vector3(10.0, 0.0, 10.0));
    {
        GLfloat matrixData[16];
        groundMatrix.getData(matrixData);
        glUniformMatrix4fv(modelShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
    }
    if (groundBuffer.begin()) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void *)(sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, groundBuffer.vertexCount());
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        groundBuffer.end();
    }
}

void drawModelBorders(Shader &shader, const Matrix4x4 &modelModifyMatrix=Matrix4x4())
{
    Matrix4x4 modelMatrix;
    modelMatrix.rotate(Vector3(1.0, 0.0, 0.0), Math::radiansFromDegrees(modelRotation.x()));
    modelMatrix.rotate(Vector3(0.0, 1.0, 0.0), Math::radiansFromDegrees(modelRotation.y()));
    modelMatrix = modelMatrix * modelModifyMatrix;
    
    for (size_t i = 0; i < modelPositions.size(); ++i) {
        Matrix4x4 modifiedModelMatrix = modelMatrix;
        modifiedModelMatrix.translate(modelPositions[i]);
        float angle = 20.0f * i;
        modifiedModelMatrix.rotate(Vector3(1.0f, 0.3f, 0.5f), Math::radiansFromDegrees(angle));
        {
            GLfloat matrixData[16];
            modifiedModelMatrix.getData(matrixData);
            glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        if (modelBorderBuffer.begin()) {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, nullptr);
            glEnableVertexAttribArray(0);
            glDrawArrays(GL_LINES, 0, modelBorderBuffer.vertexCount());
            glDisableVertexAttribArray(0);
            modelBorderBuffer.end();
        }
    }
}

void drawModels(Shader &shader, const Matrix4x4 &modelModifyMatrix=Matrix4x4()) 
{
    Matrix4x4 modelMatrix;
    modelMatrix.rotate(Vector3(1.0, 0.0, 0.0), Math::radiansFromDegrees(modelRotation.x()));
    modelMatrix.rotate(Vector3(0.0, 1.0, 0.0), Math::radiansFromDegrees(modelRotation.y()));
    modelMatrix = modelMatrix * modelModifyMatrix;
    
    for (size_t i = 0; i < modelPositions.size(); ++i) {
        Matrix4x4 modifiedModelMatrix = modelMatrix;
        modifiedModelMatrix.translate(modelPositions[i]);
        float angle = 20.0f * i;
        modifiedModelMatrix.rotate(Vector3(1.0f, 0.3f, 0.5f), Math::radiansFromDegrees(angle));
        {
            GLfloat matrixData[16];
            modifiedModelMatrix.getData(matrixData);
            glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        if (modelBuffer.begin()) {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, nullptr);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void *)(sizeof(GLfloat) * 3));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glDrawArrays(GL_TRIANGLES, 0, modelBuffer.vertexCount());
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            modelBuffer.end();
        }
    }
}

void debugShowShadowMap()
{
    // Debug show shadow map
    {
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        debugQuadShader.use();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap.textureId());

        glUniform1i(debugQuadShader.getUniformLocation("debugMap"), 0);
   
        if (quadBuffer.begin()) {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, nullptr);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (const void *)(sizeof(GLfloat) * 3));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glDrawArrays(GL_TRIANGLES, 0, quadBuffer.vertexCount());
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            quadBuffer.end();
        }
    }
}

void renderScene(double timeFactor) {
    if (lightMoveIncrease) {
        if (lightPosition.z() < lightMoveEndpoints.second)
            lightPosition.setZ(lightPosition.z() + lightMoveStep);
        else
            lightMoveIncrease = false;
    } else {
        if (lightPosition.z() > lightMoveEndpoints.first)
            lightPosition.setZ(lightPosition.z() - lightMoveStep);
        else
            lightMoveIncrease = true;
    }
    
    // Render shadow
    Matrix4x4 lightViewProjectionMatrix;
    {
        Matrix4x4 viewMatrix;
        viewMatrix.lookAt(lightPosition, Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
        
        Matrix4x4 projectionMatrix;
        projectionMatrix.orthographicProject(-10.0, 10.0, -10.0, 10.0, 1.0, 7.5);
        
        lightViewProjectionMatrix = projectionMatrix * viewMatrix;
        
        if (shadowMap.begin()) {
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE); // Disable fulling face, unless there will be hole in shadow
            {
                GLfloat matrixData[16];
                viewMatrix.getData(matrixData);
                glUniformMatrix4fv(modelShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
            }
            {
                GLfloat matrixData[16];
                projectionMatrix.getData(matrixData);
                glUniformMatrix4fv(modelShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
            }
            drawModels(shadowMap.shader());
            drawGround(shadowMap.shader());
            shadowMap.end();
        }
    }
    
    //debugShowShadowMap();
    
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_CULL_FACE);  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Matrix4x4 viewMatrix;
    viewMatrix.lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    
    Matrix4x4 projectionMatrix;
    projectionMatrix.perspectiveProject(Math::radiansFromDegrees(fov), (float)windowWidth / (float)windowHeight, 0.1, 100.0);
    
    // Draw models
    glStencilFunc(GL_ALWAYS, 1, 0xFF); 
    glStencilMask(0xFF); 
    {
        modelShader.use();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap.textureId());

        glUniform1i(modelShader.getUniformLocation("shadowMap"), 0);
        
        {
            GLfloat matrixData[16];
            viewMatrix.getData(matrixData);
            glUniformMatrix4fv(modelShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            projectionMatrix.getData(matrixData);
            glUniformMatrix4fv(modelShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            lightViewProjectionMatrix.getData(matrixData);
            glUniformMatrix4fv(modelShader.getUniformLocation("lightViewProjectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        glUniform4f(modelShader.getUniformLocation("objectColor"), 1.0, 1.0, 1.0, 1.0);
        glUniform4f(modelShader.getUniformLocation("directionLight.color"), 1.0, 1.0, 1.0, 1.0);
        glUniform4f(modelShader.getUniformLocation("directionLight.direction"), -0.2, -1.0, -0.3, 1.0);
        glUniform1f(modelShader.getUniformLocation("directionLight.ambient"), 0.05);
        glUniform1f(modelShader.getUniformLocation("directionLight.diffuse"), 0.4);
        glUniform1f(modelShader.getUniformLocation("directionLight.specular"), 0.5);
        glUniform4f(modelShader.getUniformLocation("pointLights[0].color"), 1.0, 0.0, 0.0, 1.0);
        glUniform4f(modelShader.getUniformLocation("pointLights[0].position"), lightPosition.x(), lightPosition.y(), lightPosition.z(), 1.0);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].constant"), 1.0f);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].linear"), 0.09f);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].quadratic"), 0.032f);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].ambient"), 0.05);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].diffuse"), 0.8);
        glUniform1f(modelShader.getUniformLocation("pointLights[0].specular"), 1.0);
        glUniform4f(modelShader.getUniformLocation("cameraPosition"), cameraPosition.x(), cameraPosition.y(), cameraPosition.z(), 1.0);

        drawModels(modelShader);
        glStencilMask(0x00);
        drawGround(modelShader);
        
        Matrix4x4 lightMatrix;
        lightMatrix.scale(Vector3(0.2, 0.2, 0.2));
        lightMatrix.rotate(Vector3(1.0, 0.0, 0.0), Math::radiansFromDegrees(modelRotation.x()));
        lightMatrix.rotate(Vector3(0.0, 1.0, 0.0), Math::radiansFromDegrees(modelRotation.y()));
        lightMatrix.translate(lightPosition);
        
        lightShader.use();
        {
            GLfloat matrixData[16];
            lightMatrix.getData(matrixData);
            glUniformMatrix4fv(lightShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            viewMatrix.getData(matrixData);
            glUniformMatrix4fv(lightShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            projectionMatrix.getData(matrixData);
            glUniformMatrix4fv(lightShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        glUniform4f(lightShader.getUniformLocation("lightColor"), 1.0, 0.0, 0.0, 1.0);
        if (modelBuffer.begin()) {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, nullptr);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void *)(sizeof(GLfloat) * 3));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glDrawArrays(GL_TRIANGLES, 0, modelBuffer.vertexCount());
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            modelBuffer.end();
        }
    }
    
    // Draw ray
    {
        POINT cursorPosition;
        if (GetCursorPos(&cursorPosition) && 
                ScreenToClient(windowHandle, &cursorPosition)) {
            Matrix4x4 viewProjectionMatrix;
            viewProjectionMatrix = projectionMatrix * viewMatrix;
            Vector3 nearPosition;
            Vector3 farPosition;
            if (Matrix4x4::unProjectPosition(cursorPosition.x, cursorPosition.y, 0.0,
                        (double)windowWidth, (double)windowHeight, 
                        viewProjectionMatrix,
                        nearPosition) &&
                    Matrix4x4::unProjectPosition(cursorPosition.x, cursorPosition.y, 1.0,
                        (double)windowWidth, (double)windowHeight, 
                        viewProjectionMatrix,
                        farPosition)) {
                rayCoords[0] = lightPosition[0];
                rayCoords[1] = lightPosition[1];
                rayCoords[2] = lightPosition[2];
                rayCoords[3] = 0.0;
                rayCoords[4] = 0.0;
                rayCoords[5] = 0.0;
                rayCoords[6] = nearPosition[0];
                rayCoords[7] = nearPosition[1];
                rayCoords[8] = nearPosition[2];
                rayCoords[9] = farPosition[0];
                rayCoords[10] = farPosition[1];
                rayCoords[11] = farPosition[2];
                //std::cout << nearPosition << " to " << farPosition << std::endl;
            }
        }
        
        singleColorShader.use();
        Matrix4x4 rayModelMatrix;
        {
            GLfloat matrixData[16];
            rayModelMatrix.getData(matrixData);
            glUniformMatrix4fv(singleColorShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            viewMatrix.getData(matrixData);
            glUniformMatrix4fv(singleColorShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            projectionMatrix.getData(matrixData);
            glUniformMatrix4fv(singleColorShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        glUniform4f(singleColorShader.getUniformLocation("objectColor"), 1.0, 1.0, 1.0, 1.0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, &rayCoords[0]);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, 4);
        glDisableVertexAttribArray(0);
        
        drawModelBorders(singleColorShader);
    }
    
    // Draw outline
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); 
    glDisable(GL_DEPTH_TEST);
    {
        singleColorShader.use();
        {
            GLfloat matrixData[16];
            viewMatrix.getData(matrixData);
            glUniformMatrix4fv(singleColorShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            projectionMatrix.getData(matrixData);
            glUniformMatrix4fv(singleColorShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        glUniform4f(singleColorShader.getUniformLocation("objectColor"), 0.0, 1.0, 1.0, 1.0);
        Matrix4x4 scaleMatrix;
        scaleMatrix.scale(Vector3(1.05, 1.05, 1.05));
        drawModels(singleColorShader, scaleMatrix);
    }
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);   
    glEnable(GL_DEPTH_TEST);  
    
    // Draw particles
    /*
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    {
        particleShader.use();
        
        {
            GLfloat matrixData[16];
            viewMatrix.getData(matrixData);
            glUniformMatrix4fv(particleShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
        {
            GLfloat matrixData[16];
            projectionMatrix.getData(matrixData);
            glUniformMatrix4fv(particleShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
        }
    
        particleTime += 0.001;
        if (particleTime >= 1.0f) {
            particleTime = 0.0f;
            glUniform3f(particleShader.getUniformLocation("centerPosition"), randomBetweenM0p5to0p5(randomEngine), randomBetweenM0p5to0p5(randomEngine), randomBetweenM0p5to0p5(randomEngine));
            glUniform4f(particleShader.getUniformLocation("particleColor"), randomBetween0to1(randomEngine), randomBetween0to1(randomEngine), randomBetween0to1(randomEngine), 0.5f);
        }
        // Load uniform time variable
        glUniform1f(particleShader.getUniformLocation("time"), particleTime);
        
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), &particles[0].lifetime);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), &particles[0].startPosition[0]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), &particles[0].endPosition[0]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        // Blend particles
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, smokeTextureId);
        // Set the sampler texture unit to 0
        glUniform1i(particleShader.getUniformLocation("particleMap"), 0);
        glDrawArrays(GL_POINTS, 0, particleCount);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
    */
    
    modelRotation.setY(modelRotation.y() + ((modelRotation.y() < 360.0) ? timeFactor : -360.0f + timeFactor));
}

static EGLint GetContextRenderableType ( EGLDisplay eglDisplay )
{
#ifdef EGL_KHR_create_context
    const char *extensions = eglQueryString (eglDisplay, EGL_EXTENSIONS);
    if (extensions != NULL && strstr(extensions, "EGL_KHR_create_context")) {
        return EGL_OPENGL_ES3_BIT_KHR;
    }
#endif
   return EGL_OPENGL_ES2_BIT;
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
        EGL_SAMPLE_BUFFERS, 0,
        // if EGL_KHR_create_context extension is supported, then we will use
        // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
        EGL_RENDERABLE_TYPE, GetContextRenderableType(eglDisplay),
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
	initScene();

	// Render loop

	MSG uMsg;
	PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE);
    
    LONGLONG qpcStart, qpcEnd;

	while (!quit)  {
        QueryPerformanceCounter((LARGE_INTEGER*)&qpcStart);
        
        const double cameraSpeed = 0.05;
        if (GetKeyState('A') & 0x8000) {
            cameraPosition -= Vector3::crossProduct(cameraFront, cameraUp) * cameraSpeed;
        } else if (GetKeyState('D') & 0x8000) {
            cameraPosition += Vector3::crossProduct(cameraFront, cameraUp) * cameraSpeed;
        }
        if (GetKeyState('W') & 0x8000) {
            cameraPosition += cameraFront * cameraSpeed;
        } else if (GetKeyState('S') & 0x8000) {
            cameraPosition -= cameraFront * cameraSpeed;
        }

		renderScene(timeFactor);
        glErrorLog();

		while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}

		eglSwapBuffers(eglDisplay, eglSurface);
        QueryPerformanceCounter((LARGE_INTEGER*)&qpcEnd);
		double dTime = (double)(qpcEnd - qpcStart) / (double)qpcFrequency;
		timeFactor += dTime * 0.01f;
	}
    
    return 0;
}