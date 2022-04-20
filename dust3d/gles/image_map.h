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

#ifndef DUST3D_GLES_IMAGE_MAP_H_
#define DUST3D_GLES_IMAGE_MAP_H_

#include <dust3d/base/image.h>
#include <dust3d/gles/shader.h>

namespace dust3d
{
    
class ImageMap
{
public:
    struct Cache
    {
        GLuint textureId = 0;
    };
    
    void initialize()
    {
        if (nullptr == m_shader) {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/image.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/image.frag>
                ;
            m_shader = std::unique_ptr<Shader>(new Shader(vertexShaderSource, fragmentShaderSource));
        }
    }
    
    void renderImage(const std::string &resourceName, double left, double top, double width, double height)
    {
        GLint imageTextureId = toTexture(resourceName);
        if (0 == imageTextureId)
            return;
        
        GLint lastTextureId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imageTextureId);
        glUniform1i(m_shader->getUniformLocation("imageMap"), 0);
        
        std::pair<GLfloat, GLfloat> leftBottom = {
            (GLfloat)left, 
            (GLfloat)top
        };
        std::pair<GLfloat, GLfloat> rightBottom = {
            (GLfloat)left + width, 
            (GLfloat)top
        };
        std::pair<GLfloat, GLfloat> rightTop = {
            (GLfloat)left + width, 
            (GLfloat)top + height
        };
        std::pair<GLfloat, GLfloat> leftTop = {
            (GLfloat)left, 
            (GLfloat)top + height
        };
        
        std::array<GLfloat, 16> vertices;
        size_t targetIndex = 0;

        vertices[targetIndex++] = leftBottom.first;
        vertices[targetIndex++] = leftBottom.second;
        vertices[targetIndex++] = 0.0;
        vertices[targetIndex++] = 1.0;
        
        vertices[targetIndex++] = rightBottom.first;
        vertices[targetIndex++] = rightBottom.second;
        vertices[targetIndex++] = 1.0;
        vertices[targetIndex++] = 1.0;
        
        vertices[targetIndex++] = rightTop.first;
        vertices[targetIndex++] = rightTop.second;
        vertices[targetIndex++] = 1.0;
        vertices[targetIndex++] = 0.0;
        
        vertices[targetIndex++] = leftTop.first;
        vertices[targetIndex++] = leftTop.second;
        vertices[targetIndex++] = 0.0;
        vertices[targetIndex++] = 0.0;
        
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, &vertices[0]);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, targetIndex / 4);
        glDisableVertexAttribArray(0);
        
        glBindTexture(GL_TEXTURE_2D, lastTextureId);
    }
    
    Shader &shader()
    {
        return *m_shader;
    }
    
private:
    std::map<std::string, Cache> m_caches;
    std::unique_ptr<Shader> m_shader;
    
    GLint toTexture(const std::string &resourceName)
    {
        auto findCache = m_caches.find(resourceName);
        if (findCache != m_caches.end())
            return findCache->second.textureId;
        
        Image image;
        if (!image.load(resourceName.c_str())) {
            dust3dDebug << "Load image failed, resource name:" << resourceName;
            return 0;
        }
        
        auto &cache = m_caches[resourceName];
        
        GLint lastTextureId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
        
        glGenTextures(1, &cache.textureId);
        glBindTexture(GL_TEXTURE_2D, cache.textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        
        glBindTexture(GL_TEXTURE_2D, lastTextureId);
        
        return cache.textureId;
    }
};

}

#endif

