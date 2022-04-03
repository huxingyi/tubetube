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

#ifndef DUST3D_GLES_DEPTH_MAP_H_
#define DUST3D_GLES_DEPTH_MAP_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <dust3d/base/debug.h>
#include <dust3d/gles/shader.h>

namespace dust3d
{

class DepthMap
{
public:
    void initialize()
    {
        if (0 != m_textureId)
            return;
        if (Math::isZero(m_textureHeight) || Math::isZero(m_textureWidth))
            return;
        
        if (nullptr == m_drawBuffers) {
            char *extensionString = (char *)glGetString(GL_EXTENSIONS);
            if (nullptr != strstr(extensionString, "GL_EXT_draw_buffers"))
                m_drawBuffers = (PFNGLDRAWBUFFERSEXTPROC)eglGetProcAddress("glDrawBuffersEXT");
            if (nullptr == m_drawBuffers)
                return;
        }
        
        GLenum none = GL_NONE;
        GLint defaultFramebuffer = 0;
        
        GLint lastTextureId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24_OES, m_textureWidth, m_textureHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glBindTexture(GL_TEXTURE_2D, lastTextureId);

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

        glGenFramebuffers(1, &m_frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

        m_drawBuffers(1, &none);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureId, 0);
        
        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (GL_FRAMEBUFFER_COMPLETE != status)
            std::cerr << "glCheckFramebufferStatus return:" << status << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        const GLchar *vertexShaderSource =
            #include <dust3d/gles/shaders/depth.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <dust3d/gles/shaders/depth.frag>
            ;
        m_shader = std::unique_ptr<Shader>(new Shader(vertexShaderSource, fragmentShaderSource));
    }
    
    bool begin()
    {
        if (m_sizeChanged) {
            m_sizeChanged = false;
            release();
        }
        if (0 == m_textureId) {
            initialize();
        }
        if (0 == m_textureId)
            return false;
        
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_lastFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
        glViewport(0, 0, m_textureWidth, m_textureHeight);
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(5.0f, 100.0f);
        m_shader->use();
        return true;
    }
    
    void end()
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, m_lastFramebufferId);
        m_lastFramebufferId = 0;
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    
    void release()
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        glDeleteFramebuffers(1, &m_frameBufferId);
        m_frameBufferId = 0;
    }
    
    void setSize(double width, double height)
    {
        if (Math::isEqual(width, m_textureWidth) && Math::isEqual(height, m_textureHeight))
            return;
        m_textureWidth = width;
        m_textureHeight = height;
        if (0 != m_frameBufferId)
            m_sizeChanged = true;
    }
    
    GLuint textureId() const
    {
        return m_textureId;
    }
    
    Shader &shader()
    {
        return *m_shader;
    }
    
private:
    GLuint m_textureWidth = 1024;
    GLuint m_textureHeight = 1024;
    bool m_sizeChanged = false;
    GLuint m_textureId = 0;
    GLuint m_frameBufferId = 0;
    PFNGLDRAWBUFFERSEXTPROC m_drawBuffers = nullptr;
    GLint m_lastFramebufferId = 0;
    std::unique_ptr<Shader> m_shader;
};

}

#endif
