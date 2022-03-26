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

#ifndef DUST3D_GLES_INDIE_GAME_ENGINE_H_
#define DUST3D_GLES_INDIE_GAME_ENGINE_H_

#include <string>
#include <functional>
#include <dust3d/base/debug.h>
#include <dust3d/base/matrix4x4.h>
#include <dust3d/gles/color_map.h>
#include <dust3d/gles/shader.h>
#include <dust3d/gles/vertex_buffer.h>
#include <dust3d/gles/vertex_buffer_utils.h>
#include <dust3d/gles/depth_map.h>
#include <dust3d/gles/font_map.h>

namespace dust3d
{
    
class IndieGameEngine
{
public:
    static class IndieGameEngine *indie()
    {
        static IndieGameEngine *s_indie = new IndieGameEngine;
        return s_indie;
    }
    
    enum RenderType
    {
        Default = 0x00000001,
        Ground = 0x00000002,
        Light = 0x00000004,
        AllButLight = (Default | Ground),
        All = (Default | Ground | Light)
    };
    
    enum DrawHint
    {
        Triangles = 0x00000001,
        Lines = 0x00000002,
        Texture = 0x00000004
    };
    
    class State
    {
    public:
        virtual bool update()
        {
            return false;
        }
    };
    
    class LocationState: public State
    {
    public:
        Vector3 worldLocation;
        Vector3 velocity;
        virtual bool update() override
        {
            return false;
        }
    };
    
    class Object
    {
    public:
        Object(const Object &) = delete;
        Object &operator=(const Object &) = delete;
  
        Object(const std::string &id, const std::string &resourceName, const Matrix4x4 &modelMatrix, RenderType renderType=RenderType::Default) :
            m_id(id), 
            m_resourceName(resourceName),
            m_localMatrix(modelMatrix),
            m_worldMatrix(modelMatrix),
            m_renderType(renderType)
        {
            m_vertexBufferList = indie()->createObjectVertexBufferList(resourceName);
        }
        
        ~Object()
        {
            indie()->deleteObjectVertexBufferList(m_resourceName);
        }
        
        const Matrix4x4 &worldMatrix() const
        {
            return m_worldMatrix;
        }
        
        const Matrix4x4 &localMatrix() const
        {
            return m_localMatrix;
        }
        
        void updateWorldMatrix(const Matrix4x4 &matrix)
        {
            m_worldMatrix = matrix;
        }
        
        std::vector<VertexBuffer> *vertexBufferList() const
        {
            return m_vertexBufferList;
        }
        
        RenderType renderType() const
        {
            return m_renderType;
        }
        
    private:
        RenderType m_renderType = RenderType::Default;
        Matrix4x4 m_localMatrix;
        Matrix4x4 m_worldMatrix;
        std::string m_id;
        std::string m_resourceName;
        std::vector<VertexBuffer> *m_vertexBufferList = nullptr;
    };
    
    bool addObject(const std::string &id, const std::string &resourceName, const Matrix4x4 &modelMatrix, RenderType renderType=RenderType::Default)
    {
        if (m_objects.end() != m_objects.find(id)) {
            dust3dDebug << "Add object failed, id already existed:" << id;
            return false;
        }
        m_objects.insert({id, std::make_unique<Object>(id, resourceName, modelMatrix, renderType)});
        return true;
    }
    
    Object *findObject(const std::string &id)
    {
        auto it = m_objects.find(id);
        if (m_objects.end() == it)
            return nullptr;
        return it->second.get();
    }
    
    std::vector<VertexBuffer> *createObjectVertexBufferList(const std::string &resourceName)
    {
        auto findVertexBufferList = m_vertexBufferListMap.find(resourceName);
        if (findVertexBufferList == m_vertexBufferListMap.end()) {
            if (nullptr == m_vertexBufferListLoadHander) {
                dust3dDebug << "Forgot to set vertexBufferLoadHander?";
                return nullptr;
            }
            std::unique_ptr<std::vector<VertexBuffer>> vertexBufferList = m_vertexBufferListLoadHander(resourceName);
            if (nullptr == vertexBufferList) {
                dust3dDebug << "Load resource failed, name:" << resourceName;
                return nullptr;
            }
            std::vector<VertexBuffer> *returnValue = &(*vertexBufferList);
            m_vertexBufferListMap.insert({resourceName, std::make_pair(std::move(vertexBufferList), static_cast<int64_t>(1))});
            return returnValue;
        }
        findVertexBufferList->second.second++;
        return &(*findVertexBufferList->second.first);
    }
    
    void deleteObjectVertexBufferList(const std::string &resourceName)
    {
        auto findVertexBufferList = m_vertexBufferListMap.find(resourceName);
        if (findVertexBufferList == m_vertexBufferListMap.end()) {
            dust3dDebug << "Try to delete none existed resource, name:" << resourceName;
            return;
        }
        findVertexBufferList->second.second--;
        if (findVertexBufferList->second.second <= 0) {
            m_vertexBufferListMap.erase(findVertexBufferList);
        }
    }
    
    void drawVertexBuffer(VertexBuffer &vertexBuffer)
    {
        if (vertexBuffer.begin()) {
            switch (vertexBuffer.drawHint()) {
                case DrawHint::Triangles:
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * vertexBuffer.numbersPerVertex(), nullptr);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * vertexBuffer.numbersPerVertex(), (const void *)(sizeof(GLfloat) * 3));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glDrawArrays(GL_TRIANGLES, 0, vertexBuffer.vertexCount());
                    glDisableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);
                    break;
                case DrawHint::Lines:
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * vertexBuffer.numbersPerVertex(), nullptr);
                    glEnableVertexAttribArray(0);
                    glDrawArrays(GL_LINES, 0, vertexBuffer.vertexCount());
                    glDisableVertexAttribArray(0);
                    break;
                case DrawHint::Texture:
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, nullptr);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (const void *)(sizeof(GLfloat) * 3));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexBuffer.vertexCount());
                    glDisableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);
                    break;
            }
            vertexBuffer.end();
        }
    }
    
    void renderObjects(Shader &shader, RenderType renderType, DrawHint drawHint, const Matrix4x4 *modelModifyMatrix=nullptr)
    {
        for (const auto &objectIt: m_objects) {
            const Object &object = *objectIt.second;
            if (!(object.renderType() & renderType))
                continue;
            std::vector<VertexBuffer> *vertexBufferList = object.vertexBufferList();
            if (nullptr == vertexBufferList)
                continue;
            GLfloat matrixData[16];
            if (nullptr != modelModifyMatrix) {
                Matrix4x4 matrix = object.worldMatrix() * (*modelModifyMatrix);
                matrix.getData(matrixData);
            } else {
                object.worldMatrix().getData(matrixData);
            }
            glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
            for (auto &vertexBuffer: *vertexBufferList) {
                if (!(vertexBuffer.drawHint() & drawHint))
                    continue;
                drawVertexBuffer(vertexBuffer);
            }
        }
    }
    
    void initializeScene()
    {
        if (m_initialized)
            return;
        
        glClearColor(0.145f, 0.145f, 0.145f, 1.0f);
        
        {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/phong.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/blinn-phong.frag>
                ;
            m_modelShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/phong.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/light.frag>
                ;
            m_lightShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/quad.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/debug-quad.frag>
                ;
            m_debugQuadShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/single-color.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/single-color.frag>
                ;
            m_singleColorShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/quad.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/post-processing.frag>
                ;
            m_postProcessingShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        
        std::unique_ptr<std::vector<GLfloat>> quadVertices = std::unique_ptr<std::vector<GLfloat>>(new std::vector<GLfloat> {
            -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
             1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
             1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        });
        size_t quadVertexCount = quadVertices->size() / 5;
        m_quadBuffer.update(std::move(quadVertices), 5, quadVertexCount, DrawHint::Texture);
        
        m_shadowMap.setSize(1024, 1024);
        m_shadowMap.initialize();
        m_fontMap.initialize();
        m_cameraSpaceColorMap.initialize();
        m_cameraSpaceDepthMap.initialize();
        
        m_initialized = true;
    }
    
    void renderDebugMap(GLuint textureId)
    {
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_debugQuadShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(m_debugQuadShader.getUniformLocation("debugMap"), 0);
        drawVertexBuffer(m_quadBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void flushScreen()
    {
        //renderDebugMap(m_cameraSpaceDepthMap.textureId());
        //return;
        
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_postProcessingShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_cameraSpaceColorMap.textureId());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_cameraSpaceDepthMap.textureId());
        glUniform1i(m_postProcessingShader.getUniformLocation("colorMap"), 0);
        glUniform1i(m_postProcessingShader.getUniformLocation("depthMap"), 1);
        drawVertexBuffer(m_quadBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void renderScene()
    {
        if (!m_initialized)
            initializeScene();
        
        if (m_screenIsDirty) {
            
            m_screenIsDirty = false;
        
            // Render shadow
            
            Matrix4x4 lightViewProjectionMatrix;
            {
                Matrix4x4 viewMatrix;
                viewMatrix.lookAt(m_lightPosition, Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
                
                Matrix4x4 projectionMatrix;
                projectionMatrix.orthographicProject(-10.0, 10.0, -10.0, 10.0, -10.0, 20.0);
                
                lightViewProjectionMatrix = projectionMatrix * viewMatrix;
                
                if (m_shadowMap.begin()) {
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE); // Disable fulling face, unless there will be hole in shadow
                    {
                        GLfloat matrixData[16];
                        viewMatrix.getData(matrixData);
                        glUniformMatrix4fv(m_shadowMap.shader().getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
                    }
                    {
                        GLfloat matrixData[16];
                        projectionMatrix.getData(matrixData);
                        glUniformMatrix4fv(m_shadowMap.shader().getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                    }
                    renderObjects(m_shadowMap.shader(), RenderType::AllButLight, DrawHint::Triangles);
                    m_shadowMap.end();
                }
            }
            
            Matrix4x4 viewMatrix;
            viewMatrix.lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);
            
            Matrix4x4 projectionMatrix;
            projectionMatrix.perspectiveProject(Math::radiansFromDegrees(m_fov), (float)m_windowWidth / (float)m_windowHeight, 0.1, 100.0);
            
            // Render depth
            {
                if (m_cameraSpaceDepthMap.begin()) {
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    {
                        GLfloat matrixData[16];
                        viewMatrix.getData(matrixData);
                        glUniformMatrix4fv(m_cameraSpaceDepthMap.shader().getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
                    }
                    {
                        GLfloat matrixData[16];
                        projectionMatrix.getData(matrixData);
                        glUniformMatrix4fv(m_cameraSpaceDepthMap.shader().getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                    }
                    renderObjects(m_cameraSpaceDepthMap.shader(), RenderType::AllButLight, DrawHint::Triangles);
                    m_cameraSpaceDepthMap.end();
                }
            }

            if (m_cameraSpaceColorMap.begin()) {
                
                // Render triangles
                
                glEnable(GL_STENCIL_TEST);
                glEnable(GL_DEPTH_TEST);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                glEnable(GL_CULL_FACE);  
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                m_modelShader.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_shadowMap.textureId());
                glUniform1i(m_modelShader.getUniformLocation("shadowMap"), 0);
                {
                    GLfloat matrixData[16];
                    viewMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_modelShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                {
                    GLfloat matrixData[16];
                    projectionMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_modelShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                {
                    GLfloat matrixData[16];
                    lightViewProjectionMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_modelShader.getUniformLocation("lightViewProjectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                glUniform4f(m_modelShader.getUniformLocation("objectColor"), 1.0, 1.0, 1.0, 1.0);
                glUniform4f(m_modelShader.getUniformLocation("directionLight.color"), 1.0, 1.0, 1.0, 1.0);
                glUniform4f(m_modelShader.getUniformLocation("directionLight.direction"), -0.2, -1.0, -0.3, 1.0);
                glUniform1f(m_modelShader.getUniformLocation("directionLight.ambient"), 0.05);
                glUniform1f(m_modelShader.getUniformLocation("directionLight.diffuse"), 0.4);
                glUniform1f(m_modelShader.getUniformLocation("directionLight.specular"), 0.5);
                glUniform4f(m_modelShader.getUniformLocation("pointLights[0].color"), 0xfc / 255.0, 0x66 / 255.0, 0x21 / 255.0, 1.0);
                glUniform4f(m_modelShader.getUniformLocation("pointLights[0].position"), m_lightPosition.x(), m_lightPosition.y(), m_lightPosition.z(), 1.0);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].constant"), 1.0f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].linear"), 0.09f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].quadratic"), 0.032f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].ambient"), 0.05);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].diffuse"), 0.8);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[0].specular"), 1.0);
                glUniform4f(m_modelShader.getUniformLocation("pointLights[1].color"), 0.0, 0.0, 0.0, 1.0);
                glUniform4f(m_modelShader.getUniformLocation("pointLights[1].position"), m_lightPosition.x(), m_lightPosition.y(), m_lightPosition.z(), 1.0);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].constant"), 1.0f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].linear"), 0.09f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].quadratic"), 0.032f);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].ambient"), 0.05);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].diffuse"), 0.8);
                glUniform1f(m_modelShader.getUniformLocation("pointLights[1].specular"), 1.0);
                glUniform4f(m_modelShader.getUniformLocation("cameraPosition"), m_cameraPosition.x(), m_cameraPosition.y(), m_cameraPosition.z(), 1.0);
                glStencilFunc(GL_ALWAYS, 1, 0xFF); 
                glStencilMask(0xFF);
                renderObjects(m_modelShader, RenderType::Default, DrawHint::Triangles);
                glStencilMask(0x00);
                renderObjects(m_modelShader, RenderType::Ground, DrawHint::Triangles);
                
                // Render lines
                
                m_singleColorShader.use();
                Matrix4x4 rayModelMatrix;
                {
                    GLfloat matrixData[16];
                    rayModelMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_singleColorShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                {
                    GLfloat matrixData[16];
                    viewMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_singleColorShader.getUniformLocation("viewMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                {
                    GLfloat matrixData[16];
                    projectionMatrix.getData(matrixData);
                    glUniformMatrix4fv(m_singleColorShader.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                }
                glUniform4f(m_singleColorShader.getUniformLocation("objectColor"), 0.0, 0.0, 0.0, 1.0);
                renderObjects(m_singleColorShader, RenderType::Default, DrawHint::Lines);
                
                // Render text
                
                m_fontMap.shader().use();
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_fontMap.textureId());
                glUniform1i(m_fontMap.shader().getUniformLocation("fontMap"), 0);
                {
                    Matrix4x4 projectionMatrix;
                    projectionMatrix.orthographicProject(0.0, m_windowWidth, 0.0, m_windowHeight);
                    {
                        GLfloat matrixData[16];
                        projectionMatrix.getData(matrixData);
                        glUniformMatrix4fv(m_fontMap.shader().getUniformLocation("projectionMatrix"), 1, GL_FALSE, &matrixData[0]);
                    }
                }
                glUniform4f(m_fontMap.shader().getUniformLocation("objectColor"), 1.0, 1.0, 1.0, 1.0);
                m_fontMap.renderString("Hello IndieGameEngine!", m_windowWidth / 2.0, m_windowHeight / 2.0);
                glBindTexture(GL_TEXTURE_2D, 0);
                
                m_cameraSpaceColorMap.end();
            }
        }
        
        flushScreen();
        
        GLuint glError = glGetError();
        if (glError != GL_NO_ERROR)
            std::cerr << "OpenGL Error : " << glError << std::endl;
    }
    
    void handleMouseMove(double x, double y)
    {
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
            m_cameraFront = m_cameraFront.rotated(Vector3(0.0, 1.0, 0.0), Math::radiansFromDegrees(45.0 * -offsetX / m_windowWidth));
            dirty();
        }
        if (0 != offsetY) {
            m_cameraFront = m_cameraFront.rotated(Vector3(1.0, 0.0, 0.0), Math::radiansFromDegrees(45.0 * -offsetY / m_windowHeight));
            dirty();
        }
        s_lastX = x;
        s_lastY = y;
    }

    void setVertexBufferListLoadHandler(std::function<std::unique_ptr<std::vector<VertexBuffer>>(const std::string &resourceName)> vertexBufferListLoadHander)
    {
        m_vertexBufferListLoadHander = vertexBufferListLoadHander;
    }
    
    void setMillisecondsQueryHandler(std::function<uint64_t ()> millisecondsQueryHandler)
    {
        m_millisecondsQueryHandler = millisecondsQueryHandler;
    }
    
    void setWindowSize(double width, double height)
    {
        m_windowWidth = width;
        m_windowHeight = height;
        m_cameraSpaceColorMap.setSize(m_windowWidth, m_windowHeight);
        m_cameraSpaceDepthMap.setSize(m_windowWidth, m_windowHeight);
    }
    
    void setKeyPressedQueryHandler(std::function<bool (char key)> keyPressedQueryHander)
    {
        m_keyPressedQueryHander = keyPressedQueryHander;
    }
    
    void dirty()
    {
        m_screenIsDirty = true;
    }
    
    const double &elapsedSecondsSinceLastUpdate() const
    {
        return m_elapsedSeconds;
    }

    void update()
    {
        uint64_t milliseconds = m_millisecondsQueryHandler();
        if (m_lastMilliseconds > 0)
            m_elapsedSeconds = (double)(milliseconds - m_lastMilliseconds) / 1000.0;
        m_lastMilliseconds = milliseconds;

        if (nullptr != m_keyPressedQueryHander) {
            const double cameraSpeed = 0.05;
            if (m_keyPressedQueryHander('A')) {
                m_cameraPosition -= Vector3::crossProduct(m_cameraFront, m_cameraUp) * cameraSpeed;
                dirty();
            } else if (m_keyPressedQueryHander('D')) {
                m_cameraPosition += Vector3::crossProduct(m_cameraFront, m_cameraUp) * cameraSpeed;
                dirty();
            }
            if (m_keyPressedQueryHander('W')) {
                m_cameraPosition += m_cameraFront * cameraSpeed;
                dirty();
            } else if (m_keyPressedQueryHander('S')) {
                m_cameraPosition -= m_cameraFront * cameraSpeed;
                dirty();
            }
        }
        
        for (auto &stateIt: m_generalStates) {
            if (stateIt.second->update())
                dirty();
        }
        for (auto &stateIt: m_locationStates) {
            if (stateIt.second->update()) {
                Object *object = findObject(stateIt.first);
                if (nullptr == object)
                    continue;
                Matrix4x4 translationMatrix;
                translationMatrix.translate(stateIt.second->worldLocation);
                object->updateWorldMatrix(translationMatrix * object->localMatrix());
                dirty();
            }
        }
        
        m_lastMilliseconds = milliseconds;
    }
    
    void addLocationState(const std::string &objectId, std::unique_ptr<LocationState> state)
    {
        m_locationStates[objectId] = std::move(state);
    }
    
    void addGeneralState(const std::string &objectId, std::unique_ptr<State> state)
    {
        m_generalStates[objectId] = std::move(state);
    }
    
    size_t objectCount()
    {
        return m_objects.size();
    }
    
private:
    std::function<std::unique_ptr<std::vector<VertexBuffer>> (const std::string &resourceName)> m_vertexBufferListLoadHander = nullptr;
    std::function<uint64_t ()> m_millisecondsQueryHandler = nullptr;
    std::function<bool (char key)> m_keyPressedQueryHander = nullptr;
    bool m_initialized = false;
    Vector3 m_cameraPosition = Vector3(0.0, 0.5, 3.0);
    Vector3 m_cameraFront = Vector3(0.0, 0.0, -1.0);
    Vector3 m_cameraUp = Vector3(0.0, 1.0, 0.0);
    Vector3 m_lightPosition = Vector3(0.2, 1.0, 2.0);
    double m_fov = 45.0;
    double m_windowWidth = std::numeric_limits<double>::epsilon();
    double m_windowHeight = std::numeric_limits<double>::epsilon();
    Shader m_modelShader;
    Shader m_singleColorShader;
    Shader m_lightShader;
    Shader m_debugQuadShader;
    Shader m_postProcessingShader;
    VertexBuffer m_quadBuffer;
    DepthMap m_shadowMap;
    DepthMap m_cameraSpaceDepthMap;
    FontMap m_fontMap;
    ColorMap m_cameraSpaceColorMap;
    uint64_t m_lastMilliseconds = 0;
    double m_elapsedSeconds = 0.0;
    bool m_screenIsDirty = true;
    std::map<std::string, std::pair<std::unique_ptr<std::vector<VertexBuffer>>, int64_t/*referencingCount*/>> m_vertexBufferListMap;
    std::map<std::string, std::unique_ptr<Object>> m_objects;
    std::map<std::string, std::unique_ptr<LocationState>> m_locationStates;
    std::map<std::string, std::unique_ptr<State>> m_generalStates;
};
    
};

#endif
