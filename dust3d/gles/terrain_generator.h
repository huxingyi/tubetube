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

#ifndef DUST3D_GLES_TERRAIN_GENERATOR_H_
#define DUST3D_GLES_TERRAIN_GENERATOR_H_

#include <dust3d/gles/perlin_noise.h>
#include <third_party/tga_utils/tga_utils.h>

namespace dust3d
{
    
class TerrainGenerator
{
public:
    void setWorldSize(size_t width, size_t height)
    {
        m_worldWidth = width;
        m_worldHeight = height;
    }
    
    std::unique_ptr<std::vector<double>> generateRealLayer(double offsetX, double offsetY, double scale)
    {
        auto reals = std::make_unique<std::vector<double>>();
        reals->reserve(m_worldWidth * m_worldHeight);
        for (size_t y = 0; y < m_worldHeight; ++y)
            for (size_t x = 0; x < m_worldWidth; ++x)
                reals->push_back(PerlinNoise::noise(offsetX + (double)x / m_worldWidth * scale, offsetY + (double)y / m_worldHeight * scale, 0.0));
        normalizeLayer(*reals);
        return std::move(reals);
    }
    
    static std::unique_ptr<std::vector<double>> sumLayers(const std::vector<const std::vector<double> *> &realLayers)
    {
        if (realLayers.empty())
            return nullptr;
        auto reals = std::make_unique<std::vector<double>>(realLayers.front()->size());
        for (const auto &layer: realLayers)
            for (size_t i = 0; i < layer->size(); ++i)
                reals->at(i) += layer->at(i);
        return std::move(reals);
    }
    
    static std::unique_ptr<std::vector<double>> multiplyLayers(const std::vector<const std::vector<double> *> &realLayers)
    {
        if (realLayers.empty())
            return nullptr;
        auto reals = std::make_unique<std::vector<double>>(realLayers.front()->size(), 1.0);
        for (const auto &layer: realLayers)
            for (size_t i = 0; i < layer->size(); ++i)
                reals->at(i) *= layer->at(i);
        return std::move(reals);
    }
    
    static std::vector<double> &normalizeLayer(std::vector<double> &reals)
    {
        double maxReal = *std::max_element(reals.begin(), reals.end());
        double minReal = *std::min_element(reals.begin(), reals.end());
        double range = maxReal - minReal;
        for (auto &it: reals)
            it = (it - minReal) / range;
        return reals;
    }
    
    static std::vector<double> &sqrtLayer(std::vector<double> &reals, std::function<bool (double)> filter=nullptr)
    {
        for (auto &it: reals) {
            if (nullptr == filter || filter(it))
                it = std::sqrt(it);
        }
        return reals;
    }
    
    static std::vector<double> &powLayer(std::vector<double> &reals, double exponent, std::function<bool (double)> filter=nullptr)
    {
        for (auto &it: reals) {
            if (nullptr == filter || filter(it))
                it = std::pow(it, exponent);
        }
        return reals;
    }
    
    static std::vector<double> &multiplyLayer(std::vector<double> &reals, double by, std::function<bool (double)> filter=nullptr)
    {
        for (auto &it: reals) {
            if (nullptr == filter || filter(it))
                it *= by;
        }
        return reals;
    }
    
    static std::vector<double> &originalLayer(std::vector<double> &reals)
    {
        return reals;
    }
    
    void raiseLayerCenter(std::vector<double> &reals)
    {
        int centerX = m_worldWidth / 2;
        int centerY = m_worldHeight / 2;
        double maxDistance = std::sqrt(centerX * centerX + centerY * centerY);
        size_t realIndex = 0;
        for (int y = 0; y < (int)m_worldHeight; ++y) {
            for (int x = 0; x < (int)m_worldWidth; ++x) {
                double offsetX = x - centerX;
                double offsetY = y - centerY;
                double distance = std::sqrt(offsetX * offsetX + offsetY * offsetY);
                reals[realIndex++] *= 1.0 - 0.9 * distance / maxDistance;
            }
        }
    }
    
    std::unique_ptr<TGAImage> getImage()
    {
        if (nullptr == m_heights)
            return nullptr;
        
        auto image = std::make_unique<TGAImage>();
        image->width = m_worldWidth;
        image->height = m_worldHeight;
        image->data.reserve(m_heights->size());
        for (size_t i = 0; i < m_heights->size(); ++i) {
            double value = std::min(m_heights->at(i), 1.0);
            Byte4 pixel;
            if (value >= 0.75) {
                pixel[0] = 0.0;
                pixel[1] = 255 * value;
                pixel[2] = 255 * value;
            } else if (value >= 0.6) {
                pixel[0] = 0.0;
                pixel[1] = 255 * value;
                pixel[2] = 0.0;
            } else if (value >= 0.5) {
                pixel[0] = 255 * value;
                pixel[1] = 255 * value;
                pixel[2] = 0.0;
            } else if (value >= 0.4) {
                pixel[0] = 0.0;
                pixel[1] = 0.0;
                pixel[2] = 255 * value;
            } else {
                pixel[0] = value;
                pixel[1] = value;
                pixel[2] = value;
            }
            
            pixel[3] = 255;
            image->data.push_back(pixel);
        }
        return std::move(image);
    }
    
    void generate(double frequency)
    {
        if (0 == m_worldWidth || 0 == m_worldHeight)
            return;
        
        auto layer1 = generateRealLayer(-0.5, -0.5, frequency);
        auto layer2 = generateRealLayer(-0.5, -0.5, frequency * 2.0);
        auto layer3 = generateRealLayer(-0.5, -0.5, frequency * 4.0);
        auto layer4 = sumLayers({
            &multiplyLayer(*layer1, 1.0), 
            &multiplyLayer(*layer1, 0.4),
            &multiplyLayer(*layer1, 0.1)
        });
        sqrtLayer(multiplyLayer(*layer4, 0.8));
        auto layer5 = generateRealLayer(0.0, 0.0, frequency * 3.0);
        auto layer6 = sumLayers({
            &multiplyLayer(*layer4, 0.9), 
            &multiplyLayer(*layer5, 0.1)
        });
        raiseLayerCenter(*layer6);
        normalizeLayer(*layer6);
        multiplyLayer(*layer6, 2.0, [](double n) {
            return n > 0.5;
        });
        powLayer(*layer6, 1.77, [](double n) {
            return n > 0.5;
        });
        normalizeLayer(*layer6);
        m_heights = std::move(layer6);
    }
    
    void getMesh(std::vector<Vector3> &vertices, std::vector<std::vector<size_t>> &quads, int gridSize=10, double scale=20.0, double heightRange=1.0, double heightOffset = -1.0)
    {
        size_t columns = (m_worldWidth + gridSize - 1) / gridSize;
        size_t rows = (m_worldHeight + gridSize - 1) / gridSize;
        vertices.resize(columns * rows);
        double halfWidth = (double)m_worldWidth * 0.5;
        double halfHeight = (double)m_worldHeight * 0.5;
        for (int y = 0; y < (int)m_worldHeight; y += gridSize) {
            for (int x = 0; x < (int)m_worldWidth; x += gridSize) {
                size_t intIndex = (y / gridSize) * columns + (x / gridSize);
                size_t realIndex = y * m_worldWidth + x;
                vertices[intIndex] = Vector3(scale * ((double)x - halfWidth) / m_worldWidth, heightOffset + heightRange * m_heights->at(realIndex), scale * ((double)y - halfHeight) / m_worldHeight);
            }
        }
        
        quads.clear();
        for (int y = gridSize; y < (int)m_worldHeight; y += gridSize) {
            for (int x = gridSize; x < (int)m_worldWidth; x += gridSize) {
                size_t column = x / gridSize;
                size_t row = y / gridSize;
                quads.push_back({
                    (row - 1) * columns + (column - 1),
                    (row - 1) * columns + column,
                    row * columns + column,
                    row * columns + (column - 1),
                });
            }
        }
    }
    
private:
    size_t m_worldWidth = 1024;
    size_t m_worldHeight = 1024;
    std::unique_ptr<std::vector<double>> m_heights;
};
    
}

#endif
