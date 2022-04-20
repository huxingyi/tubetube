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

#ifndef DUST3D_BASE_IMAGE_H_
#define DUST3D_BASE_IMAGE_H_

#include <dust3d/base/debug.h>
#include <third_party/stb/stb_image.h>

namespace dust3d
{
    
class Image
{
public:
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
            dust3dDebug << "stbi_load failed, filename:" << path;
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
            dust3dDebug << "stbi_load_from_memory failed";
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
    
    size_t width() const
    {
        return m_width;
    }
    
    size_t height() const
    {
        return m_height;
    }
    
private:
    size_t m_width = 0;
    size_t m_height = 0;
    unsigned char *m_data = nullptr;
};
    
}

#endif
