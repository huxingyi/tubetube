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

#ifndef HU_GLES_INDIE_GAME_ENGINE_H_
#define HU_GLES_INDIE_GAME_ENGINE_H_

#include <string>
#include <functional>
#include <hu/base/color.h>
#include <hu/base/debug.h>
#include <hu/base/matrix4x4.h>
#include <hu/base/quaternion.h>
#include <hu/base/task.h>
#include <hu/base/task_list.h>
#include <hu/base/signal.h>
#include <hu/widget/widget.h>
#include <hu/widget/push_button.h>
#include <hu/widget/radio_button.h>
#include <hu/widget/text.h>
#include <hu/gles/color_map.h>
#include <hu/gles/shader.h>
#include <hu/gles/vertex_buffer.h>
#include <hu/gles/vertex_buffer_utils.h>
#include <hu/gles/depth_map.h>
#include <hu/gles/font_map.h>
#include <hu/gles/icon_map.h>
#include <hu/gles/image_map.h>
#include <hu/gles/particles.h>

namespace Hu
{
    
class IndieGameEngine
{
public:
    Signal<> windowSizeChanged;
    Signal<> shouldPopupMenu;
    
    enum RenderType
    {
        Default = 0x00000001,
        Ground = 0x00000002,
        Light = 0x00000004,
        Water = 0x00000008,
        Terrain = (Ground | Water),
        AllButLight = (Default | Ground | Water),
        AllButLightAndWater = (Default | Ground),
        All = (Default | Ground | Light | Water)
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
        State(IndieGameEngine &engine) :
            m_engine(engine)
        {
        }
        
        virtual bool update()
        {
            return false;
        }
        
        IndieGameEngine &engine()
        {
            return m_engine;
        }
        
    private:
        IndieGameEngine &m_engine;
    };
    
    class LocationState: public State
    {
    public:
        LocationState(IndieGameEngine &engine) :
            State(engine)
        {
        }
        
        Vector3 worldLocation;
        Vector3 forwardDirection;
        Vector3 upDirection = Vector3(0.0, 1.0, 0.0);
        double speed = 0.0;
        bool followedByCamera = false;
        
        Vector3 velocity() const
        {
            return forwardDirection * speed;
        }
        
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
  
        Object(IndieGameEngine &engine, const std::string &id, const std::string &resourceName, const Matrix4x4 &modelMatrix, RenderType renderType=RenderType::Default) :
            m_engine(engine),
            m_id(id), 
            m_resourceName(resourceName),
            m_localMatrix(modelMatrix),
            m_worldMatrix(modelMatrix),
            m_renderType(renderType)
        {
            m_vertexBufferList = m_engine.createObjectVertexBufferList(resourceName);
        }
        
        ~Object()
        {
            m_engine.deleteObjectVertexBufferList(m_resourceName);
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
        IndieGameEngine &m_engine;
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
            huDebug << "Add object failed, id already existed:" << id;
            return false;
        }
        m_objects.insert({id, std::make_unique<Object>(*this, id, resourceName, modelMatrix, renderType)});
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
                huDebug << "Forgot to set vertexBufferLoadHander?";
                return nullptr;
            }
            std::unique_ptr<std::vector<VertexBuffer>> vertexBufferList = m_vertexBufferListLoadHander(resourceName);
            if (nullptr == vertexBufferList) {
                huDebug << "Load resource failed, name:" << resourceName;
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
            huDebug << "Try to delete none existed resource, name:" << resourceName;
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
                    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * vertexBuffer.numbersPerVertex(), (const void *)(sizeof(GLfloat) * 6));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glEnableVertexAttribArray(2);
                    glDrawArrays(GL_TRIANGLES, 0, vertexBuffer.vertexCount());
                    glDisableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);
                    glDisableVertexAttribArray(2);
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
    
    void initialize()
    {
        if (m_initialized)
            return;

        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/phong.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/blinn-phong.frag>
                ;
            m_modelShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/phong.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/light.frag>
                ;
            m_lightShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/quad.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/quad.frag>
                ;
            m_quadShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/single-color.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/single-color.frag>
                ;
            m_singleColorShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/quad.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/post-processing.frag>
                ;
            m_postProcessingShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/position.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/position.frag>
                ;
            m_positionShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/id.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/id.frag>
                ;
            m_idShader = Shader(vertexShaderSource, fragmentShaderSource);
        }
        {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/frame.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/frame.frag>
                ;
            m_frameShader = Shader(vertexShaderSource, fragmentShaderSource);
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
        m_fontMap.setFont("OpenSans_Condensed-Regular.ttf");
        m_iconMap.initialize();
        m_iconMap.setIconBitmapSize(16);
        m_imageMap.initialize();
        m_particles.initialize();
        m_cameraSpaceColorMap.initialize();
        m_positionMap.setSamples(1);
        m_positionMap.initialize();
        m_idMap.setSamples(1);
        m_idMap.initialize();
        m_uiMap.initialize();
        m_cameraSpaceDepthMap.initialize();

        m_initialized = true;
    }
    
    void renderDebugMap(GLuint textureId)
    {
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_quadShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(m_quadShader.getUniformLocation("colorMap"), 0);
        drawVertexBuffer(m_quadBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void flushScreen()
    {
        //renderDebugMap(m_uiMap.textureId());
        //return;
        
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_postProcessingShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_cameraSpaceColorMap.textureId());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_cameraSpaceDepthMap.textureId());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_uiMap.textureId());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_positionMap.textureId());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_idMap.textureId());
        glUniform1i(m_postProcessingShader.getUniformLocation("colorMap"), 0);
        glUniform1i(m_postProcessingShader.getUniformLocation("depthMap"), 1);
        glUniform1i(m_postProcessingShader.getUniformLocation("uiMap"), 2);
        glUniform1i(m_postProcessingShader.getUniformLocation("positionMap"), 3);
        glUniform1i(m_postProcessingShader.getUniformLocation("idMap"), 4);
        glUniform1f(m_postProcessingShader.getUniformLocation("time"), (float)m_time);
        drawVertexBuffer(m_quadBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void renderFrame(double left, double top, double width, double height, double cornerRadius=0.0)
    {
        //std::cout << "    left:" << left << " top:" << top << " width:" << width << " height:" << height << std::endl;
        
        std::array<GLfloat, 8> vertices;
        size_t targetIndex = 0;
        
        double bottom = m_windowHeight - top;
        double right = left + width;
        top = bottom - height;

        vertices[targetIndex++] = left;
        vertices[targetIndex++] = top;
        
        vertices[targetIndex++] = right;
        vertices[targetIndex++] = top;
        
        vertices[targetIndex++] = right;
        vertices[targetIndex++] = bottom;
        
        vertices[targetIndex++] = left;
        vertices[targetIndex++] = bottom;
        
        glUniform4f(m_frameShader.getUniformLocation("frameCoords"), left, top, right, bottom);
        glUniform1f(m_frameShader.getUniformLocation("frameCornerRadius"), cornerRadius);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, &vertices[0]);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, targetIndex / 2);
        glDisableVertexAttribArray(0);
    }
    
    void renderString(const std::string &string, double left, double top, double width, double height)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fontMap.textureId());
        glUniform1i(m_fontMap.shader().getUniformLocation("fontMap"), 0);
        m_fontMap.renderString(string, left, m_windowHeight - (top + height), height);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void renderIcon(const std::string &icon, double left, double top, double height)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_iconMap.textureId());
        glUniform1i(m_iconMap.shader().getUniformLocation("iconMap"), 0);
        m_iconMap.renderSvg(icon, left, m_windowHeight - (top + height), height, height);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void renderWidget(Widget *widget)
    {
        //std::cout << "renderWidget name:" << widget->name() << " color:" << widget->backgroundColor().toString() << std::endl;
        
        // Render background
        glBlendFunc(GL_ONE, GL_ZERO);
        m_frameShader.use();
        m_frameShader.setUniformColor("objectColor", widget->backgroundColor());
        if (widget->backgroundColor().alpha() > 0) {
            if (Widget::RenderHint::RadioButton & widget->renderHints()) {
                double radioLeft = widget->layoutLeft();
                double radioSize = widget->layoutHeight() * 0.7;
                double radioTop = widget->layoutTop() + (widget->layoutHeight() - radioSize) * 0.5;
                double borderSize = 2.0;
                renderFrame(radioLeft, radioTop, radioSize, radioSize);
                
                m_frameShader.setUniformColor("objectColor", widget->parentColor());
                renderFrame(radioLeft + borderSize, radioTop + borderSize, radioSize - borderSize * 2.0, radioSize - borderSize * 2.0);
                
                m_frameShader.setUniformColor("objectColor", widget->backgroundColor());
                RadioButton *button = dynamic_cast<RadioButton *>(widget);
                if (button->checked()) {
                    double marginSize = radioSize * 0.3;
                    renderFrame(radioLeft + marginSize, radioTop + marginSize, radioSize - (marginSize * 2.0), radioSize - (marginSize * 2.0));
                }
            } else {
                double radius = 0.0;
                if (Widget::RenderHint::Container & widget->renderHints())
                    radius = 8.0;
                else if (Widget::RenderHint::Element & widget->renderHints())
                    radius = 4.0;
                renderFrame(widget->layoutLeft(), widget->layoutTop(), widget->layoutWidth(), widget->layoutHeight(), radius);
            }
        }
        
        const auto &backgroundImageResourceName = widget->backgroundImageResourceName();
        if (!backgroundImageResourceName.empty()) {
            m_imageMap.shader().use();
            glUniform1f(m_imageMap.shader().getUniformLocation("opacity"), widget->backgroundImageOpacity());
            m_imageMap.renderImage(backgroundImageResourceName, widget->layoutLeft(), m_windowHeight - (widget->layoutTop() + widget->layoutHeight()), widget->layoutWidth(), widget->layoutHeight());
        }
        
        // Render push button
        if (Widget::RenderHint::PushButton & widget->renderHints()) {
            PushButton *button = dynamic_cast<PushButton *>(widget);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            double padding = widget->layoutHeight() * 0.3;
            double leftOffset = widget->paddingLeft();
            if (!button->icon().empty()) {
                double iconSize = widget->layoutHeight();
                m_iconMap.shader().use();
                m_iconMap.shader().setUniformColor("objectColor", widget->color());
                renderIcon(button->icon(), widget->layoutLeft() + leftOffset, widget->layoutTop() + (widget->layoutHeight() - iconSize) * 0.5, iconSize);
                leftOffset += iconSize + padding;
            }
            m_fontMap.shader().use();
            m_fontMap.shader().setUniformColor("objectColor", widget->color());
            renderString(button->text(), widget->layoutLeft() + leftOffset, widget->layoutTop() + widget->paddingTop(), widget->layoutWidth(), widget->layoutHeight() - widget->paddingHeight());
        }
        
        // Render radio button text
        if (Widget::RenderHint::RadioButton & widget->renderHints()) {
            RadioButton *button = dynamic_cast<RadioButton *>(widget);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            m_fontMap.shader().use();
            m_fontMap.shader().setUniformColor("objectColor", widget->color());
            renderString(button->text(), widget->layoutLeft() + widget->paddingLeft() + widget->layoutHeight(), widget->layoutTop() + widget->paddingTop(), widget->layoutWidth() - widget->layoutHeight(), widget->layoutHeight() - widget->paddingHeight());
        }
        
        // Render text
        if (Widget::RenderHint::Text & widget->renderHints()) {
            Text *text = dynamic_cast<Text *>(widget);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            m_fontMap.shader().use();
            m_fontMap.shader().setUniformColor("objectColor", widget->color());
            renderString(text->text(), widget->layoutLeft() + widget->paddingLeft(), widget->layoutTop() + widget->paddingTop(), widget->layoutWidth(), widget->layoutHeight() - widget->paddingHeight());
        }
        
        for (auto &child: widget->children())
            renderWidget(child);
    }
    
    void renderScene()
    {
        if (!m_initialized)
            initialize();
        
        bool particlesIsDirty = false;
        if (m_particles.aliveElementCount() > 0) {
            particlesIsDirty = true;
            m_screenIsDirty = true;
        }
        
        if (m_uiTaskList.anyWorkDone())
            m_screenIsDirty = true;
        
        if (Widget::layoutChanged() || Widget::m_appearanceChanged) {
            m_screenIsDirty = true;
            Widget::m_appearanceChanged = false;
        }
        
        if (m_screenIsDirty) {
            
            m_screenIsDirty = false;

            // Render shadow
            
            Matrix4x4 lightViewProjectionMatrix;
            {
                Matrix4x4 viewMatrix;
                viewMatrix.lookAt(m_lightPosition, Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
                
                Matrix4x4 shadowProjectionMatrix;
                shadowProjectionMatrix.orthographicProject(-10.0, 10.0, -10.0, 10.0, -10.0, 20.0);
                
                lightViewProjectionMatrix = shadowProjectionMatrix * viewMatrix;
                
                if (m_shadowMap.begin()) {
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE); // Disable fulling face, unless there will be hole in shadow
                    m_shadowMap.shader().setUniformMatrix("viewMatrix", viewMatrix);
                    m_shadowMap.shader().setUniformMatrix("projectionMatrix", shadowProjectionMatrix);
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
                    m_cameraSpaceDepthMap.shader().setUniformMatrix("viewMatrix", viewMatrix);
                    m_cameraSpaceDepthMap.shader().setUniformMatrix("projectionMatrix", projectionMatrix);
                    renderObjects(m_cameraSpaceDepthMap.shader(), RenderType::AllButLight, DrawHint::Triangles);
                    m_cameraSpaceDepthMap.end();
                }
            }
            
            if (m_positionMap.begin()) {
                glEnable(GL_DEPTH_TEST);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
                Matrix4x4 positionMatrix;
                m_positionShader.use();
                m_positionShader.setUniformMatrix("positionMatrix", positionMatrix);
                m_positionShader.setUniformMatrix("viewMatrix", viewMatrix);
                m_positionShader.setUniformMatrix("projectionMatrix", projectionMatrix);
                renderObjects(m_positionShader, RenderType::Default, DrawHint::Triangles);
                renderObjects(m_positionShader, RenderType::Ground, DrawHint::Triangles);
                positionMatrix.scale(Vector3(-1000, 0.0, -1000.0));
                m_positionShader.setUniformMatrix("positionMatrix", positionMatrix);
                renderObjects(m_positionShader, RenderType::Water, DrawHint::Triangles);
                m_positionMap.end();
            }
            
            if (m_idMap.begin()) {
                glEnable(GL_DEPTH_TEST);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
                m_idShader.use();
                m_idShader.setUniformMatrix("positionMatrix", Matrix4x4());
                m_idShader.setUniformMatrix("viewMatrix", viewMatrix);
                m_idShader.setUniformMatrix("projectionMatrix", projectionMatrix);
                glUniform4f(m_idShader.getUniformLocation("id"), 0.0, 0.0, 0.0, 1.0);
                renderObjects(m_idShader, RenderType::Default, DrawHint::Triangles);
                glUniform4f(m_idShader.getUniformLocation("id"), 0.1, 0.0, 0.0, 1.0);
                renderObjects(m_idShader, RenderType::Ground, DrawHint::Triangles);
                glUniform4f(m_idShader.getUniformLocation("id"), 0.2, 0.0, 0.0, 1.0);
                renderObjects(m_idShader, RenderType::Water, DrawHint::Triangles);
                m_idMap.end();
            }

            if (m_cameraSpaceColorMap.begin()) {
                
                //glClearColor(0.145f, 0.145f, 0.145f, 1.0f);
                //glClearColor(1.0f, 0.94f, 0.86f, 1.0f);
                glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], m_backgroundColor[3]);
                
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
                m_modelShader.setUniformMatrix("viewMatrix", viewMatrix);
                m_modelShader.setUniformMatrix("projectionMatrix", projectionMatrix);
                m_modelShader.setUniformMatrix("lightViewProjectionMatrix", lightViewProjectionMatrix);
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
                renderObjects(m_modelShader, RenderType::Terrain, DrawHint::Triangles);
                
                // Render partices
                
                if (particlesIsDirty) {
                    m_particles.shader().use();
                    m_particles.shader().setUniformMatrix("viewMatrix", viewMatrix);
                    m_particles.shader().setUniformMatrix("projectionMatrix", projectionMatrix);
                    glUniform1f(m_particles.shader().getUniformLocation("time"), (float)m_time);
                    glUniform2f(m_particles.shader().getUniformLocation("windowSize"), (float)m_windowWidth, (float)m_windowHeight);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particles::Element), &m_particles.elements()[0].timeRangeAndRadius[0]);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particles::Element), &m_particles.elements()[0].startPosition[0]);
                    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particles::Element), &m_particles.elements()[0].velocity[0]);
                    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particles::Element), &m_particles.elements()[0].startColor[0]);
                    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Particles::Element), &m_particles.elements()[0].stopColor[0]);
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glEnableVertexAttribArray(2);
                    glEnableVertexAttribArray(3);
                    glEnableVertexAttribArray(4);
                    glDrawArrays(GL_POINTS, 0, m_particles.elements().size());
                    glDisableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);
                    glDisableVertexAttribArray(2);
                    glDisableVertexAttribArray(3);
                    glDisableVertexAttribArray(4);
                }
                
                // Render lines
                if (m_showWireframes) {
                    m_singleColorShader.use();
                    Matrix4x4 rayModelMatrix;
                    m_singleColorShader.setUniformMatrix("modelMatrix", rayModelMatrix);
                    m_singleColorShader.setUniformMatrix("viewMatrix", viewMatrix);
                    m_singleColorShader.setUniformMatrix("projectionMatrix", projectionMatrix);
                    glUniform4f(m_singleColorShader.getUniformLocation("objectColor"), 0.0, 0.0, 0.0, 1.0);
                    renderObjects(m_singleColorShader, RenderType::Default, DrawHint::Lines);
                    renderObjects(m_singleColorShader, RenderType::Terrain, DrawHint::Lines);
                }
                
                m_cameraSpaceColorMap.end();
            }
            
            if (m_uiMap.begin()) {
                
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

                // Render text
                
                m_fontMap.shader().use();

                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ZERO);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_fontMap.textureId());
                m_fontMap.shader().setUniformMatrix("projectionMatrix", m_screenProjectionMatrix);
                //glUniform4f(m_fontMap.shader().getUniformLocation("objectColor"), 1.0, 0.0, 0.0, 1.0);
                
                m_uiTaskList.update();

                //m_fontMap.renderString(particlesIsDirty ? "Partices rendered:[" + std::to_string(m_particles.aliveElementCount()) + "]" : "Partices NOT rendered", m_windowWidth / 2.0, m_windowHeight / 2.0);
                
                glBindTexture(GL_TEXTURE_2D, 0);
                
                if (nullptr != m_rootWidget) {
                    m_iconMap.shader().use();
                    m_iconMap.shader().setUniformMatrix("projectionMatrix", m_screenProjectionMatrix);
                    m_frameShader.use();
                    m_frameShader.setUniformMatrix("projectionMatrix", m_screenProjectionMatrix);
                    m_imageMap.shader().use();
                    m_imageMap.shader().setUniformMatrix("projectionMatrix", m_screenProjectionMatrix);
                    if (Widget::layoutChanged()) {
                        m_rootWidget->layout();
                        windowSizeChanged.emit();
                    }
                    renderWidget(m_rootWidget.get());
                }

                m_uiMap.end();
            }
        }
        
        flushScreen();
        
        GLuint glError = glGetError();
        if (glError != GL_NO_ERROR)
            std::cerr << "OpenGL Error : " << glError << std::endl;
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
        m_uiMap.setSize(m_windowWidth, m_windowHeight);
        m_cameraSpaceDepthMap.setSize(m_windowWidth, m_windowHeight);
        m_positionMap.setSize(m_windowWidth, m_windowHeight);
        m_idMap.setSize(m_windowWidth, m_windowHeight);
        m_rootWidget->setSizePolicy(Widget::FixedSize);
        m_rootWidget->setSize(m_windowWidth, m_windowHeight);
        m_screenProjectionMatrix = Matrix4x4();
        m_screenProjectionMatrix.orthographicProject(0.0, m_windowWidth, 0.0, m_windowHeight);
        dirty();
    }
    
    void handleMouseMove(double x, double y)
    {
        if (nullptr != m_rootWidget)
            m_rootWidget->handleMouseMove(x, y);
    }
    
    void handleMouseLeftButtonDown()
    {
        if (nullptr != m_rootWidget)
            m_rootWidget->handleMouseLeftButtonDown();
    }
    
    void handleMouseLeftButtonUp()
    {
        if (nullptr != m_rootWidget)
            m_rootWidget->handleMouseLeftButtonUp();
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
    
    const uint64_t &millisecondsSinceStart() const
    {
        return m_millisecondsSinceStart;
    }

    void update()
    {
        uint64_t milliseconds = m_millisecondsQueryHandler();
        if (m_lastMilliseconds > 0)
            m_elapsedSeconds = (double)(milliseconds - m_lastMilliseconds) / 1000.0;
        else
            m_startMilliseconds = milliseconds - 1; // Make m_millisecondsSinceStart no zero
        m_millisecondsSinceStart = milliseconds - m_startMilliseconds;
        m_lastMilliseconds = milliseconds;
        m_time = (double)milliseconds / 1000.0;
        
        if (m_cameraPosition != m_nextCameraPosition ||
                m_cameraFront != m_nextCameraFront)
        {
            m_cameraPosition = m_nextCameraPosition;
            const double t = elapsedSecondsSinceLastUpdate();
            Matrix4x4 matrix;
            matrix.rotate(Quaternion::slerp(Quaternion(), Quaternion::rotationTo(m_cameraFront, m_nextCameraFront), t));
            m_cameraFront = matrix * m_cameraFront;
            dirty();
        }
        
        for (auto &stateIt: m_generalStates) {
            if (stateIt.second->update())
                dirty();
        }
        for (auto &stateIt: m_locationStates) {
            if (stateIt.second->update()) {
                if (stateIt.second->followedByCamera) {
                    Vector3 axis = Vector3::crossProduct(m_cameraUp, stateIt.second->forwardDirection);
                    m_nextCameraFront = stateIt.second->forwardDirection.rotated(axis, m_cameraFollowAngle).normalized();
                    m_nextCameraPosition = stateIt.second->worldLocation - m_nextCameraFront * m_cameraFollowBehindDistance;
                }
                Object *object = findObject(stateIt.first);
                if (nullptr == object)
                    continue;
                Matrix4x4 translationMatrix;
                translationMatrix.translate(stateIt.second->worldLocation);
                Matrix4x4 rotationMatrix;
                rotationMatrix.rotate(Quaternion::rotationTo(Vector3(0.0, 1.0, 0.0), stateIt.second->upDirection));
                rotationMatrix.rotate(Quaternion::rotationTo(Vector3(0.0, 0.0, -1.0), stateIt.second->forwardDirection));
                object->updateWorldMatrix(translationMatrix * rotationMatrix * object->localMatrix());
                dirty();
            }
        }
        
        m_particles.update((float)m_time);
        
        m_lastMilliseconds = milliseconds;
    }
    
    void addParticle(double durationInSeconds, double radius, const Vector3 &position, const Vector3 &velocity, const Vector3 &startColor, const Vector3 &stopColor)
    {
        m_particles.addElement(Particles::Element {
            (float)m_time, 
            (float)(m_time + durationInSeconds),
            (float)radius,
            (float)position.x(),
            (float)position.y(),
            (float)position.z(),
            (float)velocity.x(),
            (float)velocity.y(),
            (float)velocity.z(),
            (float)startColor.x(),
            (float)startColor.y(),
            (float)startColor.z(),
            (float)stopColor.x(),
            (float)stopColor.y(),
            (float)stopColor.z()
        });
    }
    
    void addLocationState(const std::string &objectId, std::unique_ptr<LocationState> state)
    {
        m_locationStates[objectId] = std::move(state);
    }
    
    void addGeneralState(const std::string &objectId, std::unique_ptr<State> state)
    {
        m_generalStates[objectId] = std::move(state);
    }
    
    size_t objectCount() const
    {
        return m_objects.size();
    }
    
    const Vector3 &cameraPosition() const
    {
        return m_cameraPosition;
    }
    
    const Vector3 &cameraTarget() const
    {
        return m_cameraPosition + m_cameraFront;
    }
    
    const Vector3 &cameraFront() const
    {
        return m_cameraFront;
    }
    
    const Vector3 &cameraUp() const
    {
        return m_cameraUp;
    }
    
    Widget *rootWidget() const
    {
        return m_rootWidget.get();
    }
    
    void setBackgroundColor(const Color &color)
    {
        if (m_backgroundColor == color)
            return;
        m_backgroundColor = color;
        Widget::m_appearanceChanged = true;
    }
    
    void run(std::unique_ptr<Task> task)
    {
        m_uiTaskList.post(std::move(task));
    }
    
    void run(std::function<void *(void)> work, std::function<void (void *)> after)
    {
        m_uiTaskList.post(work, after);
    }
    
    void run(std::function<void (void *)> after)
    {
        m_uiTaskList.post([](){return nullptr;}, after);
    }
    
    void setImageResource(const std::string &resourceName, size_t width, size_t height, const unsigned char *data)
    {
        m_imageMap.setImage(resourceName, width, height, data);
    }
    
    double measureFontWidth(const std::string &string, double lineHeight)
    {
        return m_fontMap.measureWidth(string, lineHeight);
    }
    
private:
    std::function<std::unique_ptr<std::vector<VertexBuffer>> (const std::string &resourceName)> m_vertexBufferListLoadHander = nullptr;
    std::function<uint64_t ()> m_millisecondsQueryHandler = nullptr;
    std::function<bool (char key)> m_keyPressedQueryHander = nullptr;
    bool m_initialized = false;
    Vector3 m_cameraPosition = Vector3(-3.0, 3.5, 15.0);
    Vector3 m_cameraFront = Vector3(0.0, 0.0, -1.0).normalized();
    Vector3 m_cameraUp = Vector3(0.0, 1.0, 0.0);
    Vector3 m_lightPosition = Vector3(0.2, 1.0, 2.0);
    Vector3 m_nextCameraPosition = m_cameraPosition;
    Vector3 m_nextCameraFront = m_cameraFront;
    double m_cameraFollowBehindDistance = 1.5;
    double m_cameraFollowAngle = Math::radiansFromDegrees(18.0);
    double m_fov = 45.0;
    double m_windowWidth = std::numeric_limits<double>::epsilon();
    double m_windowHeight = std::numeric_limits<double>::epsilon();
    Shader m_modelShader;
    Shader m_singleColorShader;
    Shader m_lightShader;
    Shader m_quadShader;
    Shader m_postProcessingShader;
    Shader m_positionShader;
    Shader m_idShader;
    Shader m_frameShader;
    Particles m_particles;
    VertexBuffer m_quadBuffer;
    DepthMap m_shadowMap;
    DepthMap m_cameraSpaceDepthMap;
    FontMap m_fontMap;
    IconMap m_iconMap;
    ImageMap m_imageMap;
    ColorMap m_cameraSpaceColorMap;
    ColorMap m_uiMap;
    ColorMap m_positionMap;
    ColorMap m_idMap;
    uint64_t m_startMilliseconds = 0;
    uint64_t m_millisecondsSinceStart = 0;
    uint64_t m_lastMilliseconds = 0;
    double m_time = 0.0;
    double m_elapsedSeconds = 0.0;
    bool m_screenIsDirty = true;
    bool m_showWireframes = false;
    Color m_backgroundColor;
    Matrix4x4 m_screenProjectionMatrix;
    
    TaskList m_uiTaskList;
    std::unique_ptr<Widget> m_rootWidget = std::make_unique<Widget>();
    std::map<std::string, std::pair<std::unique_ptr<std::vector<VertexBuffer>>, int64_t/*referencingCount*/>> m_vertexBufferListMap;
    std::map<std::string, std::unique_ptr<Object>> m_objects;
    std::map<std::string, std::unique_ptr<LocationState>> m_locationStates;
    std::map<std::string, std::unique_ptr<State>> m_generalStates;
};
    
};

#endif
