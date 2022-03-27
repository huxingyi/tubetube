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

#include <random>
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
        normalize(*reals);
        return std::move(reals);
    }
    
    static void normalize(std::vector<double> &reals)
    {
        double maxReal = *std::max_element(reals.begin(), reals.end());
        double minReal = *std::min_element(reals.begin(), reals.end());
        double range = maxReal - minReal;
        for (auto &it: reals)
            it = (it - minReal) / range;
    }
    
    std::unique_ptr<std::vector<double>> mix(const std::vector<const std::vector<double> *> &realLayers)
    {
        auto reals = std::make_unique<std::vector<double>>(m_worldWidth * m_worldHeight);
        for (const auto &layer: realLayers) {
            for (size_t i = 0; i < layer->size(); ++i)
                reals->at(i) += layer->at(i);
        }
        normalize(*reals);
        return std::move(reals);
    }
    
    std::unique_ptr<TGAImage> generate()
    {
        if (0 == m_worldWidth || 0 == m_worldHeight)
            return nullptr;
        
        auto layer1 = generateRealLayer(0.0, 0.0, 10.0);
        auto layer2 = generateRealLayer(1.0, 0.0, 30.0);
        auto layer3 = generateRealLayer(0.0, 5.0, 40.0);
        auto reals = mix({layer1.get(), layer2.get(), layer3.get()});
        auto image = std::make_unique<TGAImage>();
        image->width = m_worldWidth;
        image->height = m_worldHeight;
        image->data.reserve(reals->size());
        for (size_t i = 0; i < reals->size(); ++i) {
            unsigned char value = 255 * reals->at(i);
            Byte4 pixel;
            pixel[0] = value;
            pixel[2] = value;
            pixel[1] = value;
            pixel[3] = 255;
            image->data.push_back(pixel);
        }
        return std::move(image);
    }
    
private:
    size_t m_worldWidth = 1024;
    size_t m_worldHeight = 1024;
    std::mt19937 m_randomGenerator = std::mt19937(0);
    std::uniform_real_distribution<double> m_randomDistribution = std::uniform_real_distribution<double>(0.0, 1.0);
    std::function<double ()> m_rand = std::bind(m_randomDistribution, m_randomGenerator);
};
    
}

#endif
