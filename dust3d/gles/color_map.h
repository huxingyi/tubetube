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

#ifndef DUST3D_GLES_COLOR_MAP_H_
#define DUST3D_GLES_COLOR_MAP_H_

#include <algorithm>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>

namespace dust3d
{
    
class ColorMap
{
public:
    void initialize()
    {
        if (0 != m_resultFrameBufferId)
            return;
        if (Math::isZero(m_textureHeight) || Math::isZero(m_textureWidth))
            return;
        
        if (nullptr == m_renderbufferStorageMultisample) {
            char *extensionString = (char *)glGetString(GL_EXTENSIONS);
            if (nullptr != strstr(extensionString, "GL_ANGLE_framebuffer_multisample")) {
                m_renderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)eglGetProcAddress("glRenderbufferStorageMultisampleANGLE");
                m_blitFramebuffer = (PFNGLBLITFRAMEBUFFERANGLEPROC)eglGetProcAddress("glBlitFramebufferANGLE");
            }
            if (nullptr == m_renderbufferStorageMultisample)
                return;
        }
        
        GLint defaultFramebuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

        GLsizei maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES_ANGLE, &maxSamples);
        GLsizei numSamples = std::min(m_numSamples, maxSamples);

        glGenRenderbuffers(1, &m_sampleColorRenderBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_sampleColorRenderBufferId);
        m_renderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA8_OES, m_textureWidth, m_textureHeight);
        
        glGenFramebuffers(1, &m_sampleFrameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_sampleFrameBufferId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_sampleColorRenderBufferId);

        glGenRenderbuffers(1, &m_sampleDepthStencilRenderBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_sampleDepthStencilRenderBufferId);
        m_renderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_DEPTH24_STENCIL8_OES, m_textureWidth, m_textureHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sampleDepthStencilRenderBufferId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_sampleDepthStencilRenderBufferId);
        
        {
            auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (GL_FRAMEBUFFER_COMPLETE != status)
                std::cerr << "glCheckFramebufferStatus return:" << status << std::endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glGenFramebuffers(1, &m_resultFrameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_resultFrameBufferId);

        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);
        
        {
            auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (GL_FRAMEBUFFER_COMPLETE != status)
                std::cerr << "glCheckFramebufferStatus return:" << status << std::endl;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    }
    
    void setSize(double width, double height)
    {
        if (Math::isEqual(width, m_textureWidth) && Math::isEqual(height, m_textureHeight))
            return;
        m_textureWidth = width;
        m_textureHeight = height;
        if (0 != m_resultFrameBufferId)
            m_configureChanged = true;
    }
    
    void release()
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        
        glDeleteRenderbuffers(1, &m_sampleDepthStencilRenderBufferId);
        m_sampleDepthStencilRenderBufferId = 0;
        
        glDeleteRenderbuffers(1, &m_sampleColorRenderBufferId);
        m_sampleColorRenderBufferId = 0;
        
        glDeleteFramebuffers(1, &m_resultFrameBufferId);
        m_resultFrameBufferId = 0;
        
        glDeleteFramebuffers(1, &m_sampleFrameBufferId);
        m_sampleFrameBufferId = 0;
    }
    
    GLuint textureId()
    {
        return m_textureId;
    }
    
    bool begin()
    {
        if (m_configureChanged) {
            m_configureChanged = false;
            release();
        }
        if (0 == m_resultFrameBufferId) {
            initialize();
        }
        if (0 == m_resultFrameBufferId)
            return false;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_lastFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_sampleFrameBufferId);
        glViewport(0, 0, m_textureWidth, m_textureHeight);
        return true;
    }
    
    void end()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, m_sampleFrameBufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, m_resultFrameBufferId);
        m_blitFramebuffer(0, 0, m_textureWidth, m_textureHeight, 0, 0, m_textureWidth, m_textureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, m_lastFramebufferId);
        m_lastFramebufferId = 0;
    }
    
    void setSamples(GLsizei numSamples)
    {
        if (numSamples == m_numSamples)
            return;
        m_numSamples = numSamples;
        if (0 != m_resultFrameBufferId)
            m_configureChanged = true;
    }
    
private:
    double m_textureWidth = 0.0;
    double m_textureHeight = 0.0;
    bool m_configureChanged = false;
    GLuint m_resultFrameBufferId = 0;
    GLuint m_textureId = 0;
    GLuint m_sampleColorRenderBufferId = 0;
    GLuint m_sampleFrameBufferId = 0;
    GLuint m_sampleDepthStencilRenderBufferId = 0;
    GLint m_lastFramebufferId = 0;
    GLsizei m_numSamples = 4;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC m_renderbufferStorageMultisample = nullptr;
    PFNGLBLITFRAMEBUFFERANGLEPROC m_blitFramebuffer = nullptr;
};
    
};

#endif

