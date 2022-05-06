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

#ifndef HU_GLES_ICON_MAP_H_
#define HU_GLES_ICON_MAP_H_

#include <hu/base/debug.h>
#include <hu/gles/shader.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <nanosvg.h>
#include <nanosvgrast.h>

namespace Hu
{

class IconMap
{
public:
    struct ImageClip
    {
        std::pair<GLfloat, GLfloat> leftBottomUv;
        std::pair<GLfloat, GLfloat> rightBottomUv;
        std::pair<GLfloat, GLfloat> rightTopUv;
        std::pair<GLfloat, GLfloat> leftTopUv;
    };
    
    void initialize()
    {
        if (nullptr == m_shader) {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/icon.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/icon.frag>
                ;
            m_shader = std::unique_ptr<Shader>(new Shader(vertexShaderSource, fragmentShaderSource));
        }
        
        if (0 == m_textureId) {
            GLint lastTextureId = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
            glGenTextures(1, &m_textureId);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_EXT, m_textureWidth, m_textureHeight, 0, GL_RED_EXT, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, lastTextureId);
        }
    }
    
    GLuint textureId() const
    {
        return m_textureId;
    }
    
    void setIconBitmapSize(size_t pixelSize)
    {
        if (m_iconBitmapSize > 0) {
            if ((size_t)m_iconBitmapSize == pixelSize)
                return;
            huDebug << "Icon size could not be changed, previous value:" << m_iconBitmapSize;
            return;
        }
        m_iconBitmapSize = (int)pixelSize;
        m_columns = m_textureWidth / m_iconBitmapSize;
        m_rows = m_textureHeight / m_iconBitmapSize;
    }
    
    void renderSvg(const std::string &svgPath, double left, double top, double width, double height)
    {
        const ImageClip *imageClip = addSvgToTexture(svgPath);
        if (nullptr == imageClip) {
            huDebug << "renderSvg failed because of addSvgToTexture failure";
            return;
        }
        
        const auto &clip = *imageClip;

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

        vertices[targetIndex++] = leftBottom.first; // left bottom x
        vertices[targetIndex++] = leftBottom.second; // left bottom y
        vertices[targetIndex++] = clip.leftBottomUv.first; // left bottom u
        vertices[targetIndex++] = clip.leftBottomUv.second; // left bottom v
        
        vertices[targetIndex++] = rightBottom.first; // right bottom x
        vertices[targetIndex++] = rightBottom.second; // right bottom y
        vertices[targetIndex++] = clip.rightBottomUv.first; // right bottom u
        vertices[targetIndex++] = clip.rightBottomUv.second; // right bottom v
        
        vertices[targetIndex++] = rightTop.first; // right top x
        vertices[targetIndex++] = rightTop.second; // right top y
        vertices[targetIndex++] = clip.rightTopUv.first; // right top u
        vertices[targetIndex++] = clip.rightTopUv.second; // right top v
        
        vertices[targetIndex++] = leftTop.first; // left top x
        vertices[targetIndex++] = leftTop.second; // left top y
        vertices[targetIndex++] = clip.leftTopUv.first; // left top u
        vertices[targetIndex++] = clip.leftTopUv.second; // left top v
        
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, &vertices[0]);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, targetIndex / 4);
        glDisableVertexAttribArray(0);
    }
    
    Shader &shader()
    {
        return *m_shader;
    }
    
private:
    GLuint m_textureWidth = 1024;
    GLuint m_textureHeight = 1024;
    GLuint m_textureId = 0;
    std::unique_ptr<Shader> m_shader;
    int m_iconBitmapSize = 0;
    int m_columns = 0;
    int m_rows = 0;
    int m_currentColumn = 0;
    int m_currentRow = 0;
    std::map<std::string, ImageClip> m_imageClipMap;
    NSVGrasterizer *m_svgRasterizer = nullptr;
    
    const ImageClip *addSvgToTexture(const std::string &svgPath)
    {
        if (0 == m_iconBitmapSize) {
            huDebug << "addSvgToTexture failed because of bitmap size is zero";
            return nullptr;
        }
        
        auto findImageClip = m_imageClipMap.find(svgPath);
        if (findImageClip != m_imageClipMap.end())
            return &findImageClip->second;
        
        if (m_currentRow >= m_rows) {
            huDebug << "addSvgToTexture failed because of texture space is used up";
            return nullptr;
        }
        
        if (nullptr == m_svgRasterizer) {
            m_svgRasterizer = nsvgCreateRasterizer();
            if (nullptr == m_svgRasterizer) {
                huDebug << "addSvgToTexture failed because of nsvgCreateRasterizer failure";
                return nullptr;
            }
        }
        NSVGimage *image = nsvgParseFromFile(svgPath.c_str(), "px", 96);
        if (nullptr == image) {
            huDebug << "addSvgToTexture failed because of nsvgParseFromFile failure";
            return nullptr;
        }
        std::vector<unsigned char> rgba(m_iconBitmapSize * m_iconBitmapSize * 4);
        nsvgRasterize(m_svgRasterizer, image, 0, 0, 1.0, rgba.data(), m_iconBitmapSize, m_iconBitmapSize, m_iconBitmapSize * 4);
        std::vector<unsigned char> a(rgba.size() / 4);
        for (size_t i = 0, j = 0; i < a.size(); ++i, j += 4)
            a[i] = rgba[j + 3];
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLint lastTextureId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        int column = m_currentColumn;
        int row = m_currentRow;
        ++m_currentColumn;
        if (m_currentColumn >= m_columns) {
            m_currentColumn = 0;
            ++m_currentRow;
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, column * m_iconBitmapSize, row * m_iconBitmapSize, m_iconBitmapSize, m_iconBitmapSize, GL_RED_EXT, GL_UNSIGNED_BYTE, a.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glBindTexture(GL_TEXTURE_2D, lastTextureId);
        
        auto insertResult = m_imageClipMap.insert({svgPath, ImageClip {
            {
                (GLfloat)(column * m_iconBitmapSize) / m_textureWidth,
                (GLfloat)(row * m_iconBitmapSize + m_iconBitmapSize) / m_textureHeight
            },
            {
                (GLfloat)(column * m_iconBitmapSize + m_iconBitmapSize) / m_textureWidth,
                (GLfloat)(row * m_iconBitmapSize + m_iconBitmapSize) / m_textureHeight
            },
            {
                (GLfloat)(column * m_iconBitmapSize + m_iconBitmapSize) / m_textureWidth,
                (GLfloat)(row * m_iconBitmapSize) / m_textureHeight
            },
            {
                (GLfloat)(column * m_iconBitmapSize) / m_textureWidth,
                (GLfloat)(row * m_iconBitmapSize) / m_textureHeight
            }
        }});

        nsvgDelete(image);
        
        return &insertResult.first->second;
    }
};
    
}

#endif
