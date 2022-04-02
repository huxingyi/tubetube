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

#ifndef DUST3D_GLES_FONT_MAP_H_
#define DUST3D_GLES_FONT_MAP_H_

#include <string>
#include <array>
#include <locale>
#include <codecvt>
#include <dust3d/base/debug.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <ft2build.h>
#include <freetype/freetype.h>

namespace dust3d
{
    
class FontMap
{
public:
    struct ImageClip
    {
        int column;
        int row;
        int bitmapLeft;
        int bitmapBottomMove;
        unsigned int bitmapWidth;
        unsigned int bitmapHeight;
        int advanceX;
        uint64_t age;
    };
    
    void initialize()
    {
        if (nullptr == m_shader) {
            const GLchar *vertexShaderSource =
                #include <dust3d/gles/shaders/font.vert>
                ;
            const GLchar *fragmentShaderSource = 
                #include <dust3d/gles/shaders/font.frag>
                ;
            m_shader = std::unique_ptr<Shader>(new Shader(vertexShaderSource, fragmentShaderSource));
        }
        
        if (nullptr == m_library) {
            auto error = FT_Init_FreeType(&m_library);
            if (error) {
                dust3dDebug << "FT_Init_FreeType failed:" << error;
                return;
            }
        }
        
        if (m_fontFilePath.empty() || 0 == m_fontSizeInPixel)
            return;
        
        m_columns = m_textureWidth / m_fontSizeInPixel;
        m_rows = m_textureHeight / m_fontSizeInPixel;
        m_nextAge = 1;
        m_currentColumn = 0;
        m_currentRow = 0;
        m_imageClipMap.clear();
        
        auto error = FT_New_Face(m_library, m_fontFilePath.c_str(), 0, &m_face);
        if (FT_Err_Unknown_File_Format == error) {
            dust3dDebug << "FT_New_Face failed: FT_Err_Unknown_File_Format";
        } else if (error) {
            dust3dDebug << "FT_New_Face failed:" << error;
        }
        
        error = FT_Set_Pixel_Sizes(m_face, 0, m_fontSizeInPixel);
        if (error) {
            dust3dDebug << "FT_Set_Pixel_Sizes failed:" << error;
        }
    }
    
    void setFont(const char *filePath, size_t pixelSize)
    {
        std::string newFontFilePath = filePath;
        if (m_fontFilePath == newFontFilePath && m_fontSizeInPixel == pixelSize)
            return;
        
        if (nullptr != m_face) {
            FT_Done_Face(m_face);
            m_face = nullptr;
        }
        m_fontFilePath = newFontFilePath;
        m_fontSizeInPixel = pixelSize;
        
        initialize();
    }
    
    void renderString(const std::string &string, double left, double top)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string utf16 = utf16conv.from_bytes(string);
        addCharsToImageClips(utf16);
        std::vector<GLfloat> vertices(24 * utf16.size());
        size_t targetIndex = 0;
        for (size_t i = 0; i < utf16.size(); ++i) {
            auto findImageClip = m_imageClipMap.find(utf16[i]);
            if (findImageClip == m_imageClipMap.end())
                continue;
            
            const auto &clip = findImageClip->second;
            
            std::pair<GLfloat, GLfloat> leftBottom = {
                (GLfloat)left + clip.bitmapLeft, 
                (GLfloat)top - clip.bitmapBottomMove
            };
            std::pair<GLfloat, GLfloat> rightBottom = {
                (GLfloat)left + clip.bitmapLeft + clip.bitmapWidth, 
                (GLfloat)top - clip.bitmapBottomMove
            };
            std::pair<GLfloat, GLfloat> rightTop = {
                (GLfloat)left + clip.bitmapLeft + clip.bitmapWidth, 
                (GLfloat)top - clip.bitmapBottomMove + clip.bitmapHeight
            };
            std::pair<GLfloat, GLfloat> leftTop = {
                (GLfloat)left + clip.bitmapLeft, 
                (GLfloat)top - clip.bitmapBottomMove + clip.bitmapHeight
            };
            
            std::pair<GLfloat, GLfloat> leftBottomUv = {
                (GLfloat)(clip.column * m_fontSizeInPixel) / m_textureWidth,
                (GLfloat)(clip.row * m_fontSizeInPixel + clip.bitmapHeight) / m_textureHeight
            };
            std::pair<GLfloat, GLfloat> rightBottomUv = {
                (GLfloat)(clip.column * m_fontSizeInPixel + clip.bitmapWidth) / m_textureWidth,
                (GLfloat)(clip.row * m_fontSizeInPixel + clip.bitmapHeight) / m_textureHeight
            };
            std::pair<GLfloat, GLfloat> rightTopUv = {
                (GLfloat)(clip.column * m_fontSizeInPixel + clip.bitmapWidth) / m_textureWidth,
                (GLfloat)(clip.row * m_fontSizeInPixel) / m_textureHeight
            };
            std::pair<GLfloat, GLfloat> leftTopUv = {
                (GLfloat)(clip.column * m_fontSizeInPixel) / m_textureWidth,
                (GLfloat)(clip.row * m_fontSizeInPixel) / m_textureHeight
            };

            vertices[targetIndex++] = leftBottom.first; // left bottom x
            vertices[targetIndex++] = leftBottom.second; // left bottom y
            vertices[targetIndex++] = leftBottomUv.first; // left bottom u
            vertices[targetIndex++] = leftBottomUv.second; // left bottom v
            
            vertices[targetIndex++] = rightBottom.first; // right bottom x
            vertices[targetIndex++] = rightBottom.second; // right bottom y
            vertices[targetIndex++] = rightBottomUv.first; // right bottom u
            vertices[targetIndex++] = rightBottomUv.second; // right bottom v
            
            vertices[targetIndex++] = rightTop.first; // right top x
            vertices[targetIndex++] = rightTop.second; // right top y
            vertices[targetIndex++] = rightTopUv.first; // right top u
            vertices[targetIndex++] = rightTopUv.second; // right top v
            
            vertices[targetIndex++] = rightTop.first; // right top x
            vertices[targetIndex++] = rightTop.second; // right top y
            vertices[targetIndex++] = rightTopUv.first; // right top u
            vertices[targetIndex++] = rightTopUv.second; // right top v
            
            vertices[targetIndex++] = leftTop.first; // left top x
            vertices[targetIndex++] = leftTop.second; // left top y
            vertices[targetIndex++] = leftTopUv.first; // left top u
            vertices[targetIndex++] = leftTopUv.second; // left top v
            
            vertices[targetIndex++] = leftBottom.first; // left bottom x
            vertices[targetIndex++] = leftBottom.second; // left bottom y
            vertices[targetIndex++] = leftBottomUv.first; // left bottom u
            vertices[targetIndex++] = leftBottomUv.second; // left bottom v
            
            left += clip.advanceX;
        }

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
    FT_Library m_library = nullptr;
    FT_Face m_face = nullptr;
    int m_fontSizeInPixel = 13;
    int m_columns = 0;
    int m_rows = 0;
    uint64_t m_nextAge = 1;
    int m_currentColumn = 0;
    int m_currentRow = 0;
    std::string m_fontFilePath;
    std::map<char16_t, ImageClip> m_imageClipMap;
    
    void addCharsToImageClips(const std::u16string &utf16)
    {
        for (size_t i = 0; i < utf16.size(); ++i) {
            auto findImageClip = m_imageClipMap.find(utf16[i]);
            if (findImageClip != m_imageClipMap.end()) {
                findImageClip->second.age = m_nextAge++;
                continue;
            }
            FT_UInt glyphIndex = FT_Get_Char_Index(m_face, utf16[i]);
            auto error = FT_Load_Glyph(m_face, glyphIndex, FT_LOAD_DEFAULT);
            if (error) {
                dust3dDebug << "FT_Load_Glyph failed:" << error;
                continue;
            }
            const auto &slot = m_face->glyph;
            error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
            if (error) {
                dust3dDebug << "FT_Render_Glyph failed:" << error;
                continue;
            }
            int column = 0;
            int row = 0;
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
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            if (0 == m_textureId) {
                glGenTextures(1, &m_textureId);
                glBindTexture(GL_TEXTURE_2D, m_textureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_EXT, m_textureWidth, m_textureHeight, 0, GL_RED_EXT, GL_UNSIGNED_BYTE, nullptr);
            } else {
                glBindTexture(GL_TEXTURE_2D, m_textureId);
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, column * m_fontSizeInPixel, row * m_fontSizeInPixel, slot->bitmap.width, slot->bitmap.rows, GL_RED_EXT, GL_UNSIGNED_BYTE, slot->bitmap.buffer);
            m_imageClipMap.insert({utf16[i], ImageClip {
                column,
                row,
                slot->bitmap_left,
                (int)slot->bitmap.rows - slot->bitmap_top,
                slot->bitmap.width,
                slot->bitmap.rows,
                slot->advance.x >> 6,
                m_nextAge++
            }});
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
};
    
};

#endif
