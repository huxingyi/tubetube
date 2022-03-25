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

#ifndef DUST3D_GLES_SCREEN_MAP_H_
#define DUST3D_GLES_SCREEN_MAP_H_

namespace dust3d
{
    
class ScreenMap
{
public:
    void initialize()
    {
        if (0 != m_frameBufferId)
            return;
        if (m_height <= 0.0 || m_width <= 0.0)
            return;
        
        GLint defaultFramebuffer = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);
        glGenFramebuffers(1, &m_frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
        
        glGenTextures(1, &m_textureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);
        
        glGenRenderbuffers(1, &m_renderBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferId);

        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (GL_FRAMEBUFFER_COMPLETE != status)
            std::cerr << "glCheckFramebufferStatus return:" << status << std::endl;
        
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    }
    
    void setSize(double width, double height)
    {
        if (Math::isEqual(width, m_width) && Math::isEqual(height, m_height))
            return;
        m_width = width;
        m_height = height;
        m_sizeChanged = true;
    }
    
    void release()
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        glDeleteRenderbuffers(1, &m_renderBufferId);
        m_renderBufferId = 0;
        glDeleteFramebuffers(1, &m_frameBufferId);
        m_frameBufferId = 0;
    }
    
    GLuint textureId()
    {
        return m_textureId;
    }
    
    bool begin()
    {
        if (m_sizeChanged) {
            m_sizeChanged = false;
            release();
        }
        if (0 == m_frameBufferId) {
            initialize();
        }
        if (0 == m_frameBufferId)
            return false;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_lastFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
        glViewport(0, 0, m_width, m_height);
        return true;
    }
    
    void end()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_lastFramebuffer);
        m_lastFramebuffer = 0;
    }
    
private:
    double m_width = 0.0;
    double m_height = 0.0;
    bool m_sizeChanged = false;
    GLuint m_frameBufferId = 0;
    GLuint m_textureId = 0;
    GLuint m_renderBufferId = 0;
    GLint m_lastFramebuffer = 0;
};
    
};

#endif

