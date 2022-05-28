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

#include <fstream>
#include <hu/base/image.h>
#include <dust3d/document/document.h>
#include <dust3d/document/ds3_file.h>

namespace Dust3d
{

void Document::save(const std::string &path)
{
    Ds3FileWriter ds3Writer;
    
    if (nullptr != m_referenceImage) {
        std::vector<uint8_t> pngBuffer;
        m_referenceImage->saveAsPng(&pngBuffer);
        ds3Writer.add("canvas.png", "asset", &pngBuffer[0], pngBuffer.size());
    }
    
    ds3Writer.save(path);
}

void Document::setReferenceImage(std::unique_ptr<Hu::Image> image)
{
    m_referenceImage = std::move(image);
    referenceImageChanged.emit();
}

void Document::open(const std::string &path)
{
    std::ifstream is(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    
    Dust3d::Ds3FileReader ds3Reader(&fileData[0], fileData.size());
    for (int i = 0; i < ds3Reader.items().size(); ++i) {
        const Dust3d::Ds3ReaderItem &item = ds3Reader.items()[i];
        if (item.type == "asset") {
            if (item.name == "canvas.png") {
                std::vector<std::uint8_t> data;
                ds3Reader.loadItem(item.name, &data);
                auto image = std::make_unique<Hu::Image>();
                image->load(data.data(), (int)data.size());
                setReferenceImage(std::move(image));
            }
        }
    }
    
    // TODO:
}

Hu::Image *Document::referenceImage()
{
    return m_referenceImage.get();
}

}
