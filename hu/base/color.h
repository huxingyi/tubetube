/*
 *  Copyright (c) 2016-2022 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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

#ifndef HU_BASE_COLOR_H_
#define HU_BASE_COLOR_H_

#include <string>
#include <iostream>
#include <algorithm>
#include <array>
#include <hu/base/debug.h>
#include <hu/base/math.h>

namespace Hu
{
    
class Color;

inline std::string to_string(const Color &color);
    
class Color
{
public:
    inline Color() :
        m_data {0.0, 0.0, 0.0, 0.0}
    {
    }
    
    inline Color(double r, double g, double b) :
        m_data {r, g, b, 1.0}
    {
    }
    
    inline Color(double r, double g, double b, double alpha) :
        m_data {r, g, b, alpha}
    {
    }
    
    inline Color(const std::string &name)
    {
        if (7 == name.size() && '#' == name[0]) {
            m_data[3] = 1.0;
            m_data[0] = strtoul(name.substr(1, 2).c_str(), nullptr, 16) / 255.0;
            m_data[1] = strtoul(name.substr(3, 2).c_str(), nullptr, 16) / 255.0;
            m_data[2] = strtoul(name.substr(5, 2).c_str(), nullptr, 16) / 255.0;
        } else if (9 == name.size() && '#' == name[0]) {
            m_data[3] = strtoul(name.substr(1, 2).c_str(), nullptr, 16) / 255.0;
            m_data[0] = strtoul(name.substr(3, 2).c_str(), nullptr, 16) / 255.0;
            m_data[1] = strtoul(name.substr(5, 2).c_str(), nullptr, 16) / 255.0;
            m_data[2] = strtoul(name.substr(7, 2).c_str(), nullptr, 16) / 255.0;
        }
    }
    
    inline double &operator[](size_t index)
    {
        return m_data[index];
    }
    
    inline const double &operator[](size_t index) const
    {
        return m_data[index];
    }

    inline const double &alpha() const
    {
        return m_data[3];
    }

    inline const double &r() const
    {
        return m_data[0];
    }
    
    inline const double &g() const
    {
        return m_data[1];
    }
    
    inline const double &b() const
    {
        return m_data[2];
    }
    
    inline const double &a() const
    {
        return m_data[3];
    }
    
    inline double &alpha()
    {
        return m_data[3];
    }

    inline double &r()
    {
        return m_data[0];
    }
    
    inline double &g()
    {
        return m_data[1];
    }
    
    inline double &b()
    {
        return m_data[2];
    }
    
    inline double &a()
    {
        return m_data[3];
    }

    inline const double &red() const
    {
        return m_data[0];
    }
    
    inline const double &green() const
    {
        return m_data[1];
    }
    
    inline const double &blue() const
    {
        return m_data[2];
    }

    inline double &red()
    {
        return m_data[0];
    }
    
    inline double &green()
    {
        return m_data[1];
    }
    
    inline double &blue()
    {
        return m_data[2];
    }
    
    inline std::string toString() const
    {
        return to_string(*this);
    }
    
    inline Color lighted(double weight=0.1) const
    {
        auto hsl = rgbToHsl({red(), green(), blue()});
        auto rgb = hslToRgb({hsl[0], hsl[1], std::min(hsl[2] + weight, 1.0)});
        return Color(rgb[0], rgb[1], rgb[2]);
    }
    
    inline Color darked(double weight=0.1) const
    {
        return lighted(-weight);
    }
    
    inline static std::array<double, 3> rgbToHsl(const std::array<double, 3> &rgb) 
    {
        double maxChannel = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
        double minChannel = std::min(std::min(rgb[0], rgb[1]), rgb[2]);
        double l = (maxChannel + minChannel) / 2.0;
        double h = l, s = l;
        if (maxChannel > minChannel) {
            double d = maxChannel - minChannel;
            s = l > 0.5 ? d / (2.0 - (maxChannel + minChannel)) : d / (maxChannel + minChannel);
            if (rgb[0] >= maxChannel) {
                h = (rgb[1] - rgb[2]) / d + (rgb[1] < rgb[2] ? 6.0 : 0.0);
            } else if (rgb[1] >= maxChannel) {
                h = (rgb[2] - rgb[0]) / d + 2.0;
            } else {
                h = (rgb[0] - rgb[1]) / d + 4.0;
            }
            h /= 6.0;
        } else {
            h = s = 0.0;
        }
        return {h, s, l};
    }
    
    inline static float hueToRgb(const std::array<double, 3> &hue)
    {
        double z = hue[2];
        if (z < 0.0)
            z += 1.0;
        if (z > 1.0)
            z -= 1.0;
        if (z < 1.0 / 6.0)
            return hue[0] + (hue[1] - hue[0]) * 6.0 * z;
        if (z < 1.0 / 2.0)
            return hue[1];
        if (z < 2.0 / 3.0)
            return hue[0] + (hue[1] - hue[0]) * (2.0 / 3.0 - z) * 6.0;
        return hue[0];
    }
    
    inline static std::array<double, 3> hslToRgb(const std::array<double, 3> &hsl) 
    {
        double r, g, b;
        if (hsl[1] > 0.0) {
            float q = hsl[2] < 0.5 ? hsl[2] * (1.0 + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];
            float p = 2.0 * hsl[2] - q;
            r = hueToRgb({p, q, hsl[0] + 1.0 / 3.0});
            g = hueToRgb({p, q, hsl[0]});
            b = hueToRgb({p, q, hsl[0] - 1.0 / 3.0});
        } else {
            r = g = b = hsl[2];
        }
        return {r, g, b};
    }

private:
    double m_data[4] = {0.0, 0.0, 0.0, 0.0};
};

inline std::string to_string(const Color &color)
{
    static const char *digits = "0123456789ABCDEF";
    std::string name = "#00000000";
    
    int alpha = static_cast<int>(color.alpha() * 255);
    int r = static_cast<int>(color.r() * 255);
    int g = static_cast<int>(color.g() * 255);
    int b = static_cast<int>(color.b() * 255);
    
    name[1] = digits[(alpha & 0xf0) >> 4];
    name[2] = digits[alpha & 0x0f];
    name[3] = digits[(r & 0xf0) >> 4];
    name[4] = digits[r & 0x0f];
    name[5] = digits[(g & 0xf0) >> 4];
    name[6] = digits[g & 0x0f];
    name[7] = digits[(b & 0xf0) >> 4];
    name[8] = digits[b & 0x0f];

    return name;
}

inline Color operator*(const Color &color, double number)
{
    return Color(number * color[0], number * color[1], number * color[2], color[3]);
}

inline Color operator/(const Color &color, double number)
{
    return Color(color[0] / number, color[1] / number, color[2] / number, color[3]);
}

inline Color operator+(const Color &a, const Color &b)
{
    return Color(a[0] + b[0], a[1] + b[1], a[2] + b[2], std::max(a[3], b[3]));
}

inline bool operator==(const Color &a, const Color &b)
{
    return Math::isEqual(a[0], b[0]) && 
        Math::isEqual(a[1], b[1]) &&
        Math::isEqual(a[2], b[2]) &&
        Math::isEqual(a[3], b[3]);
}

}

#endif
