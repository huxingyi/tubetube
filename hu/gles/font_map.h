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

#ifndef HU_GLES_FONT_MAP_H_
#define HU_GLES_FONT_MAP_H_

#include <cmath>
#include <string>
#include <array>
#include <locale>
#include <codecvt>
#include <hu/base/debug.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <msdfgen.h>
#include <msdfgen-ext.h>

namespace Hu
{
    
class FontMap
{
public:
    struct ImageClip
    {
        int column = 0;
        int row = 0;
        uint16_t utf16 = 0;
        int bitmapLeft = 0;
        int bitmapBottomMove = 0;
        unsigned int bitmapWidth = 0;
        unsigned int bitmapHeight = 0;
        double advanceX = 0.0;
        uint64_t age = 0;
        std::pair<GLfloat, GLfloat> leftBottomUv = {0.0f, 0.0f};
        std::pair<GLfloat, GLfloat> rightBottomUv = {0.0f, 0.0f};
        std::pair<GLfloat, GLfloat> rightTopUv = {0.0f, 0.0f};
        std::pair<GLfloat, GLfloat> leftTopUv = {0.0f, 0.0f};
    };
    
    void initialize()
    {
        if (nullptr == m_shader) {
            const GLchar *vertexShaderSource =
                #include <hu/gles/shaders/font.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <hu/gles/shaders/font.frag>
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            
            glBindTexture(GL_TEXTURE_2D, lastTextureId);
        }
        
        resetImageClips();
    }
    
    void setFont(const char *filePath)
    {
        std::string newFontFilePath = filePath;
        if (m_fontFilePath == newFontFilePath)
            return;
        
        if (nullptr != m_fontHandle) {
            msdfgen::destroyFont(m_fontHandle);
            m_fontHandle = nullptr;
        }
        m_fontFilePath = newFontFilePath;
        
        if (nullptr == m_freeTypeHandle)
            m_freeTypeHandle = msdfgen::initializeFreetype();
        if (nullptr == m_fontHandle) {
            m_fontHandle = msdfgen::loadFont(m_freeTypeHandle, m_fontFilePath.c_str());
            if (nullptr != m_fontHandle) {
                msdfgen::FontMetrics metrics = {0};
                getFontMetrics(metrics, m_fontHandle);
                m_fontSizeInPixel = metrics.lineHeight;
            }
        }
        
        resetImageClips();
    }
    
    double measureWidth(const std::string &string, double lineHeight)
    {
        if (0 == m_fontSizeInPixel)
            return 0.0;
        
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string utf16String = utf16conv.from_bytes(string);
        addCharsToImageClips(utf16String);
        
        std::vector<const ImageClip *> clips = utf16StringToImageClips(utf16String);
        
        double maxFontHeight = 0;
        double maxMove = 0;
        double advanceX = 0;
        for (size_t i = 0; i < clips.size(); ++i) {
            maxFontHeight = std::max(maxFontHeight, (double)clips[i]->bitmapHeight);
            maxMove = std::max(maxMove, (double)clips[i]->bitmapBottomMove);
            double kerning = 0.0;
            if (i + 1 < clips.size())
                msdfgen::getKerning(kerning, m_fontHandle, clips[i]->utf16, clips[i + 1]->utf16);
            advanceX += clips[i]->advanceX + kerning;
        }
        double scale = lineHeight / (maxFontHeight + maxMove);
        return advanceX * scale;
    }
    
    void renderString(const std::string &string, double left, double top, double lineHeight)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string utf16String = utf16conv.from_bytes(string);
        addCharsToImageClips(utf16String);
        
        std::vector<const ImageClip *> clips = utf16StringToImageClips(utf16String);
        
        double maxFontHeight = 0;
        double maxMove = 0;
        for (size_t i = 0; i < clips.size(); ++i) {
            maxFontHeight = std::max(maxFontHeight, (double)clips[i]->bitmapHeight);
            maxMove = std::max(maxMove, (double)clips[i]->bitmapBottomMove);
        }
        double scale = lineHeight / (maxFontHeight + maxMove);
        
        top += maxMove * scale;
        
        std::vector<GLfloat> vertices(24 * clips.size());
        size_t targetIndex = 0;
        for (size_t i = 0; i < clips.size(); ++i) {
            const auto &clip = *clips[i];
            
            std::pair<GLfloat, GLfloat> leftBottom = {
                (GLfloat)(left + clip.bitmapLeft * scale), 
                (GLfloat)(top - clip.bitmapBottomMove * scale)
            };
            std::pair<GLfloat, GLfloat> rightBottom = {
                (GLfloat)(left + clip.bitmapLeft * scale + clip.bitmapWidth * scale), 
                (GLfloat)(top - clip.bitmapBottomMove * scale)
            };
            std::pair<GLfloat, GLfloat> rightTop = {
                (GLfloat)(left + clip.bitmapLeft * scale + clip.bitmapWidth * scale), 
                (GLfloat)(top - clip.bitmapBottomMove * scale + clip.bitmapHeight * scale)
            };
            std::pair<GLfloat, GLfloat> leftTop = {
                (GLfloat)(left + clip.bitmapLeft * scale), 
                (GLfloat)(top - clip.bitmapBottomMove * scale + clip.bitmapHeight * scale)
            };
            
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
            
            vertices[targetIndex++] = rightTop.first; // right top x
            vertices[targetIndex++] = rightTop.second; // right top y
            vertices[targetIndex++] = clip.rightTopUv.first; // right top u
            vertices[targetIndex++] = clip.rightTopUv.second; // right top v
            
            vertices[targetIndex++] = leftTop.first; // left top x
            vertices[targetIndex++] = leftTop.second; // left top y
            vertices[targetIndex++] = clip.leftTopUv.first; // left top u
            vertices[targetIndex++] = clip.leftTopUv.second; // left top v
            
            vertices[targetIndex++] = leftBottom.first; // left bottom x
            vertices[targetIndex++] = leftBottom.second; // left bottom y
            vertices[targetIndex++] = clip.leftBottomUv.first; // left bottom u
            vertices[targetIndex++] = clip.leftBottomUv.second; // left bottom v
            
            double kerning = 0.0;
            if (i + 1 < clips.size())
                msdfgen::getKerning(kerning, m_fontHandle, clip.utf16, clips[i + 1]->utf16);
            
            std::cout << "[" << i << "] advanceX:" << clip.advanceX << " kerning:" << kerning << std::endl;
            
            left += (clip.advanceX + kerning) * scale;
        }
        
        if (0 == targetIndex)
            return;
        
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, vertices.data());
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, targetIndex / 4);
        glDisableVertexAttribArray(0);
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
    GLuint m_textureId = 0;
    std::unique_ptr<Shader> m_shader;
    int m_fontSizeInPixel = 0;
    int m_columns = 0;
    int m_rows = 0;
    uint64_t m_nextAge = 1;
    int m_currentColumn = 0;
    int m_currentRow = 0;
    std::string m_fontFilePath;
    std::map<char16_t, ImageClip> m_imageClipMap;
    msdfgen::FreetypeHandle *m_freeTypeHandle = nullptr;
    msdfgen::FontHandle *m_fontHandle = nullptr;
    
    void resetImageClips()
    {
        if (0 == m_fontSizeInPixel)
            return;
        
        m_columns = m_textureWidth / m_fontSizeInPixel;
        m_rows = m_textureHeight / m_fontSizeInPixel;
        m_nextAge = 1;
        m_currentColumn = 0;
        m_currentRow = 0;
        m_imageClipMap.clear();
    }
    
    std::vector<const ImageClip *> utf16StringToImageClips(const std::u16string &utf16String)
    {
        std::vector<const ImageClip *> clips;
        clips.reserve(utf16String.size());
        for (size_t i = 0; i < utf16String.size(); ++i) {
            auto findImageClip = m_imageClipMap.find(utf16String[i]);
            if (findImageClip == m_imageClipMap.end())
                continue;
            clips.push_back(&findImageClip->second);
        }
        return std::move(clips);
    }
    
    ImageClip *allocImageClip(char16_t utf16)
    {
        int column, row;
        if (m_currentRow >= m_rows) {
            auto minElement = std::min_element(m_imageClipMap.begin(), m_imageClipMap.end(), [](const auto &first, const auto &second) {
                return first.second.age < second.second.age;
            });
            column = minElement->second.column;
            row = minElement->second.row;
            m_imageClipMap.erase(minElement);
        } else {
            column = m_currentColumn;
            row = m_currentRow;
            ++m_currentColumn;
            if (m_currentColumn >= m_columns) {
                m_currentColumn = 0;
                ++m_currentRow;
            }
        }
        auto &clip = m_imageClipMap[utf16];
        clip.column = column;
        clip.row = row;
        clip.utf16 = utf16;
        clip.age = m_nextAge++;
        return &clip;
    }
    
    void addCharsToImageClips(const std::u16string &utf16)
    {
        for (size_t i = 0; i < utf16.size(); ++i) {
            auto findImageClip = m_imageClipMap.find(utf16[i]);
            if (findImageClip != m_imageClipMap.end()) {
                findImageClip->second.age = m_nextAge++;
                continue;
            }
            double advanceX = 0.0;
            msdfgen::Shape shape;
            if (!msdfgen::loadGlyph(shape, m_fontHandle, (msdfgen::unicode_t)utf16[i], &advanceX))
                continue;
            ImageClip *clip = allocImageClip(utf16[i]);
            if (nullptr == clip)
                continue;
            shape.normalize();
            auto bounds = shape.getBounds(3.0);
            clip->advanceX = advanceX;
            clip->bitmapLeft = bounds.l;
            clip->bitmapBottomMove = -bounds.b;
            clip->bitmapWidth = bounds.r - bounds.l;
            clip->bitmapHeight = bounds.t - bounds.b;
            clip->leftBottomUv.first = (GLfloat)(clip->column * m_fontSizeInPixel) / m_textureWidth;
            clip->leftBottomUv.second = (GLfloat)(clip->row * m_fontSizeInPixel) / m_textureHeight;
            clip->rightBottomUv.first = (GLfloat)(clip->column * m_fontSizeInPixel + clip->bitmapWidth) / m_textureWidth;
            clip->rightBottomUv.second = (GLfloat)(clip->row * m_fontSizeInPixel) / m_textureHeight;
            clip->rightTopUv.first = (GLfloat)(clip->column * m_fontSizeInPixel + clip->bitmapWidth) / m_textureWidth;
            clip->rightTopUv.second = (GLfloat)(clip->row * m_fontSizeInPixel + clip->bitmapHeight) / m_textureHeight;
            clip->leftTopUv.first = (GLfloat)(clip->column * m_fontSizeInPixel) / m_textureWidth;
            clip->leftTopUv.second = (GLfloat)(clip->row * m_fontSizeInPixel + clip->bitmapHeight) / m_textureHeight;
            msdfgen::edgeColoringSimple(shape, 3.0);
            msdfgen::Bitmap<float, 3> msdf(m_fontSizeInPixel, m_fontSizeInPixel);
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(-bounds.l, -bounds.b));
            const msdfgen::BitmapConstRef<float, 3> &bitmap = (const msdfgen::BitmapConstRef<float, 3>)msdf;
            std::vector<unsigned char> image;
            image.reserve(bitmap.width * bitmap.height * 4);
            for (int y = 0; y < bitmap.height; ++y) {
                for (int x = 0; x < bitmap.width; ++x) {
                    image.push_back(msdfgen::pixelFloatToByte(bitmap(x, y)[0]));
                    image.push_back(msdfgen::pixelFloatToByte(bitmap(x, y)[1]));
                    image.push_back(msdfgen::pixelFloatToByte(bitmap(x, y)[2]));
                    image.push_back(0x00);
                }
            }
            GLint lastTextureId = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, clip->column * m_fontSizeInPixel, clip->row * m_fontSizeInPixel, bitmap.width, bitmap.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
            glBindTexture(GL_TEXTURE_2D, lastTextureId);
        }
    }
};
    
}

#endif
