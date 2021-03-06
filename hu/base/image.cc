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

#include <hu/base/image.h>
#define STB_IMAGE_IMPLEMENTATION
#include <third_party/stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <third_party/stb/stb_image_write.h>

namespace Hu
{
    
static void writePngToBuffer(void *context, void *data, int length)
{
   std::vector<uint8_t> *buffer = (std::vector<uint8_t> *)context;
   buffer->resize(length);
   memcpy(&(*buffer)[0], data, length);
}

void Image::saveAsPng(std::vector<uint8_t> *buffer)
{
    stbi_write_png_to_func(writePngToBuffer, buffer, m_width, m_height, 4, m_data, m_width * 4);
}

}
