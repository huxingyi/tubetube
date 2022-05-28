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

#ifndef HU_BASE_IMAGE_H_
#define HU_BASE_IMAGE_H_

#include <vector>
#include <hu/base/debug.h>
#include <third_party/stb/stb_image.h>
#include <third_party/stb/stb_image_write.h>

namespace Hu
{
    
class Image
{
public:
    Image() = default;
    
    Image(const Image &other):
        m_width(other.width()),
        m_height(other.height())
    {
        size_t dataSize = m_width * m_height * 4;
        m_data = (unsigned char *)malloc(dataSize);
        memcpy(m_data, other.data(), dataSize);
    }
    
    Image(Image &&other)
    {
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_data, other.m_data);
    }
    
    Image(size_t width, size_t height):
        m_width(width),
        m_height(height)
    {
        m_data = (unsigned char *)malloc(width * height * 4);
    }
    
    void clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        size_t offset = 0;
        for (size_t y = 0; y < m_height; ++y) {
            for (size_t x = 0; x < m_width; ++x) {
                m_data[offset++] = r;
                m_data[offset++] = g;
                m_data[offset++] = b;
                m_data[offset++] = a;
            }
        }
    }
    
    Image *scaledToHeight(size_t toHeight)
    {
        size_t toWidth = m_width * (double)toHeight / m_height;
        Image *toImage = new Image(toWidth, toHeight);
        unsigned char *toData = toImage->data();
        double scaleX = (double)m_width / toWidth;
        double scaleY = (double)m_height / toHeight;
        for (double y = 0; y < toHeight; y += 1.0) {
            size_t srcY = std::min((size_t)(scaleY * y), m_height - 1);
            for (double x = 0; x < toWidth; x += 1.0) {
                size_t srcX = std::min((size_t)(scaleX * x), m_width - 1);
                size_t srcOffset = (size_t)(srcY * m_width + srcX) * 4;
                size_t targetOffset = (size_t)(y * toWidth + x) * 4;
                memcpy(toData + targetOffset, m_data + srcOffset, 4);
            }
        }
        return toImage;
    }
    
    void copy(const Image &source, size_t sourceLeft, size_t sourceTop, size_t targetLeft, size_t targetTop, size_t width, size_t height)
    {
        const unsigned char *sourceData = source.data();
        size_t sourceCopyHeight = sourceTop >= source.height() ? 0 : std::min(height, source.height() - sourceTop);
        size_t sourceCopyWidth = sourceLeft >= source.width() ? 0 : std::min(width, source.width() - sourceLeft);
        size_t targetCopyHeight = targetTop >= this->height() ? 0 : std::min(height, this->height() - targetTop);
        size_t targetCopyWidth = targetLeft >= this->width() ? 0 : std::min(width, this->width() - targetLeft);
        height = std::min(sourceCopyHeight, targetCopyHeight);
        width = std::min(sourceCopyWidth, targetCopyWidth);
        for (size_t y = 0; y < height; ++y) {
            size_t sourceOffset = ((sourceTop + y) * source.width() + sourceLeft) * 4;
            size_t targetOffset = ((targetTop + y) * this->width() + targetLeft) * 4;
            for (size_t x = 0; x < width; ++x) {
                memcpy(&m_data[targetOffset], &sourceData[sourceOffset], 4);
                targetOffset += 4;
                sourceOffset += 4;
            }
        }
    }

    ~Image()
    {
        release();
    }
    
    void release()
    {
        m_width = 0;
        m_height = 0;
        stbi_image_free(m_data);
        m_data = nullptr;
    }
    
    bool load(const char *path)
    {
        int width = 0;
        int height = 0;
        int numComponents = 0;
        unsigned char *imageData = stbi_load(path, &width, &height, &numComponents, 4);
        if (nullptr == imageData) {
            huDebug << "stbi_load failed, filename:" << path;
            return false;
        }
        
        release();
        m_width = width;
        m_height = height;
        m_data = imageData;
        
        return true;
    }
    
    bool load(const unsigned char *fileContent, int fileSize)
    {
        int width = 0;
        int height = 0;
        int numComponents = 0;
        unsigned char *imageData = stbi_load_from_memory(fileContent, fileSize, &width, &height, &numComponents, 4);
        if (nullptr == imageData) {
            huDebug << "stbi_load_from_memory failed";
            return false;
        }
        
        release();
        m_width = width;
        m_height = height;
        m_data = imageData;
        
        return true;
    }

    const unsigned char *data() const
    {
        return m_data;
    }
    
    unsigned char *data()
    {
        return m_data;
    }
    
    size_t width() const
    {
        return m_width;
    }
    
    size_t height() const
    {
        return m_height;
    }
    
    void saveAsPng(std::vector<uint8_t> *buffer);
    
private:
    size_t m_width = 0;
    size_t m_height = 0;
    unsigned char *m_data = nullptr;
};
    
}

#endif
