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
 *
 *  Translated from Perlin's Java code:
 *  JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE - COPYRIGHT 2002 KEN PERLIN.
 *  https://cs.nyu.edu/~perlin/noise/
 *
 */

#ifndef HU_GLES_PERLIN_NOISE_H_
#define HU_GLES_PERLIN_NOISE_H_

#include <cmath>
#include <array>

namespace Hu
{
    
#define PERLIN_PERMUTATION  151, 160, 137, 91, 90, 15,                                                  \
        131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,  \
        190,  6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,   \
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168,  68, 175, 74, 165, 71, 134, 139, 48, 27, 166,    \
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,    \
        102, 143, 54,  65, 25, 63, 161,  1, 216, 80, 73, 209, 76, 132, 187, 208,  89, 18, 169, 200, 196,    \
        135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,  3, 64, 52, 217, 226, 250, 124, 123,     \
        5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,   \
        223, 183, 170, 213, 119, 248, 152,  2, 44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172, 9,     \
        129, 22, 39, 253,  19, 98, 108, 110, 79, 113, 224, 232, 178, 185,  112, 104, 218, 246, 97, 228,     \
        251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,  81, 51, 145, 235, 249, 14, 239, 107,     \
        49, 192, 214,  31, 181, 199, 106, 157, 184,  84, 204, 176, 115, 121, 50, 45, 127,  4, 150, 254,     \
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180

static std::array<int, 512> PerlinNoiseP = {
    PERLIN_PERMUTATION,
    PERLIN_PERMUTATION
};
    
class PerlinNoise
{
public:
    static double noise(double x, double y, double z)
    {
        int X = (int)std::floor(x) & 255;
        int Y = (int)std::floor(y) & 255;
        int Z = (int)std::floor(z) & 255;
        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);
        double u = fade(x);
        double v = fade(y);
        double w = fade(z);
        int A  = PerlinNoiseP[X] + Y;
        int AA = PerlinNoiseP[A] + Z;
        int AB = PerlinNoiseP[A + 1] + Z;
        int B  = PerlinNoiseP[X + 1] + Y;
        int BA = PerlinNoiseP[B] + Z;
        int BB = PerlinNoiseP[B + 1] + Z;
        return lerp(w, lerp(v, lerp(u, grad(PerlinNoiseP[AA], x, y, z),
                                       grad(PerlinNoiseP[BA], x - 1, y, z)),
                               lerp(u, grad(PerlinNoiseP[AB], x, y - 1, z),
                                       grad(PerlinNoiseP[BB], x - 1, y - 1, z))),
                       lerp(v, lerp(u, grad(PerlinNoiseP[AA + 1], x, y, z - 1),
                                       grad(PerlinNoiseP[BA + 1], x - 1, y, z - 1)),
                               lerp(u, grad(PerlinNoiseP[AB + 1], x, y - 1, z - 1),
                                       grad(PerlinNoiseP[BB + 1], x - 1, y - 1, z - 1))));
    }
    
    static double fade(double t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    
    static double lerp(double t, double a, double b) 
    {
        return a + t * (b - a);
    }
    
    static double grad(int hash, double x, double y, double z)
    {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};
    
};

#endif
