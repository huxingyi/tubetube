/*
 *  Copyright (c) 2016-2021 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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

#ifndef DUST3D_BASE_MATRIX4X4_H_
#define DUST3D_BASE_MATRIX4X4_H_

#include <memory>
#include <algorithm>
#include <array>
#include <dust3d/base/vector3.h>
#include <dust3d/base/quaternion.h>

namespace dust3d
{
    
class Matrix4x4;
    
inline Vector3 operator*(const Matrix4x4 &m, const Vector3 &v);
inline Matrix4x4 operator*(const Matrix4x4 &a, const Matrix4x4 &b);
inline std::array<double, 4> operator*(const Matrix4x4 &m, const std::array<double, 4> &v);

class Matrix4x4
{
public:
    // | 0  4  8  12 |
    // | 1  5  9  13 |
    // | 2  6  10 14 |
    // | 3  7  11 15 |
    
    Matrix4x4()
    {
    }
    
    Matrix4x4(const std::array<double, 16> &dataInColumnMajor)
    {
        memcpy(m_data, &dataInColumnMajor[0], sizeof(m_data));
    }

    Matrix4x4(const Matrix4x4 &other)
    {
        memcpy(m_data, other.constData(), sizeof(m_data));
    }
    
    double operator()(size_t col, size_t row) const 
    {
        return m_data[col * 4 + row];
    }
    
    double &operator()(size_t col, size_t row) 
    {
        return m_data[col * 4 + row];
    }
    
    inline void rotate(const Vector3 &axis, double angle)
    {
        double cosine = std::cos(angle);
        double sine = std::sin(angle);
        double oneMinusCosine = 1.0 - cosine;
        Matrix4x4 by;
        double *byData = by.data();
        byData[0] = cosine + axis.x() * axis.x() * oneMinusCosine;
        byData[4] = axis.x() * axis.y() * oneMinusCosine - axis.z() * sine;
        byData[8] = axis.x() * axis.z() * oneMinusCosine + axis.y() * sine;
        byData[12] = 0.0;
        byData[1] = axis.y() * axis.x() * oneMinusCosine + axis.z() * sine;
        byData[5] = cosine + axis.y() * axis.y() * oneMinusCosine;
        byData[9] = axis.y() * axis.z() * oneMinusCosine - axis.x() * sine;
        byData[13] = 0.0;
        byData[2] = axis.z() * axis.x() * oneMinusCosine - axis.y() * sine;
        byData[6] = axis.z() * axis.y() * oneMinusCosine + axis.x() * sine;
        byData[10] = cosine + axis.z() * axis.z() * oneMinusCosine;
        byData[14] = 0.0;
        byData[3] = 0.0;
        byData[7] = 0.0;
        byData[11] = 0.0;
        byData[15] = 1.0;
        *this *= by;
    }
    
    inline const double *constData() const
    {
        return &m_data[0];
    }
    
    inline double *data()
    {
        return &m_data[0];
    }
    
    inline void getData(float data[16]) const
    {
        for (size_t i = 0; i < 16; ++i)
            data[i] = m_data[i];
    }
    
    inline void lookAt(const Vector3 &eyePosition, const Vector3 &targetPosition, const Vector3 &upDirection)
    {
        Vector3 zAxis = (eyePosition - targetPosition).normalized();
        Vector3 xAxis = Vector3::crossProduct(upDirection, zAxis).normalized();
        Vector3 yAxis = Vector3::crossProduct(zAxis, xAxis);
        Matrix4x4 tmp;
        double *tmpData = tmp.data();
        tmpData[0] = xAxis[0];
        tmpData[1] = yAxis[0];
        tmpData[2] = zAxis[0];
        tmpData[4] = xAxis[1];
        tmpData[5] = yAxis[1];
        tmpData[6] = zAxis[1];
        tmpData[8] = xAxis[2];
        tmpData[9] = yAxis[2];
        tmpData[10] = zAxis[2];
        tmpData[12] = -Vector3::dotProduct(xAxis, eyePosition);
        tmpData[13] = -Vector3::dotProduct(yAxis, eyePosition);
        tmpData[14] = -Vector3::dotProduct(zAxis, eyePosition);
        *this *= tmp;
    }
    
    static inline bool unProjectPosition(double screenX, double screenY, double zPlane,
        double screenWidth, double screenHeight, 
        const Matrix4x4 &viewProjectionMatrix,
        Vector3 &position3d)
    {
        bool invertSucceed = false;
        Matrix4x4 inverseMatrix = viewProjectionMatrix.inverted(&invertSucceed);
        if (!invertSucceed)
            return false;
        std::array<double, 4> position({
            screenX / screenWidth * 2.0 - 1.0,
            (screenHeight - screenY) / screenHeight * 2.0 - 1.0,
            zPlane * 2.0 - 1.0,
            1.0
        });
        if (Math::isZero(position[3]))
            return false;
        position[3] = 1.0 / position[3];
        position = inverseMatrix * position;
        position3d[0] = position[0] * position[3];
        position3d[1] = position[1] * position[3];
        position3d[2] = position[2] * position[3];
        return true;
    }
    
    inline void perspectiveProject(double fov, double aspect, double nearPlane, double farPlane)
    {
        double yScale = 1.0 / std::tan(fov / 2.0);
        double xScale = yScale / aspect;
        double farMinusNear = farPlane - nearPlane;
        Matrix4x4 tmp;
        double *tmpData = tmp.data();
        tmpData[0] = xScale;
        tmpData[5] = yScale;
        tmpData[10] = -(farPlane + nearPlane) / farMinusNear;
        tmpData[11] = -1.0; 
        tmpData[14] = -2.0 * farPlane * nearPlane / farMinusNear;
        tmpData[15] = 0.0;
        *this *= tmp;
    }
    
    inline void orthographicProject(double left, double right, double bottom, double top, double nearPlane, double farPlane)
    {
        Matrix4x4 tmp;
        double *tmpData = tmp.data();
        tmpData[0] = 2.0 / (right - left);
        tmpData[5] = 2.0 / (top - bottom);
        tmpData[10] = -2.0 / (farPlane - nearPlane);
        tmpData[12] = -(right + left) / (right - left);
        tmpData[13] = -(top + bottom) / (top - bottom);
        tmpData[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        *this *= tmp;
    }
    
    inline void rotate(const Quaternion &q)
    {
        Matrix4x4 tmp;
        double *tmpData = tmp.data();
        double xx = q.x() + q.x();
        double yy = q.y() + q.y();
        double zz = q.z() + q.z();
        double xxw = xx * q.w();
        double yyw = yy * q.w();
        double zzw = zz * q.w();
        double xxx = xx * q.x();
        double xxy = xx * q.y();
        double xxz = xx * q.z();
        double yyy = yy * q.y();
        double yyz = yy * q.z();
        double zzz = zz * q.z();
        tmpData[0] = 1.0f - (yyy + zzz);
        tmpData[4] =         xxy - zzw;
        tmpData[8] =         xxz + yyw;
        tmpData[12] = 0.0f;
        tmpData[1] =         xxy + zzw;
        tmpData[5] = 1.0f - (xxx + zzz);
        tmpData[9] =         yyz - xxw;
        tmpData[13] = 0.0f;
        tmpData[2] =         xxz - yyw;
        tmpData[6] =         yyz + xxw;
        tmpData[10] = 1.0f - (xxx + yyy);
        tmpData[14] = 0.0f;
        tmpData[3] = 0.0f;
        tmpData[7] = 0.0f;
        tmpData[11] = 0.0f;
        tmpData[15] = 1.0f;
        *this *= tmp;
    }
    
    inline Matrix4x4 &scale(const Vector3 &v)
    {
        m_data[0] *= v[0];
        m_data[5] *= v[1];
        m_data[10] *= v[2];
        return *this;
    }
    
    inline Matrix4x4 &translate(const Vector3 &v)
    {
        m_data[12] += v.x();
        m_data[13] += v.y();
        m_data[14] += v.z();
        return *this;
    }

    inline Matrix4x4 inverted(bool *succeed=nullptr) const
    {
        // Inverted implementation translated from https://github.com/willnode/N-Matrix-Programmer (Wildan Mubarok, MIT License)
        
        const auto &m = *this;
        
        double A2323 = m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2);
        double A1323 = m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1);
        double A1223 = m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1);
        double A0323 = m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0);
        double A0223 = m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0);
        double A0123 = m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0);
        double A2313 = m(1, 2) * m(3, 3) - m(1, 3) * m(3, 2);
        double A1313 = m(1, 1) * m(3, 3) - m(1, 3) * m(3, 1);
        double A1213 = m(1, 1) * m(3, 2) - m(1, 2) * m(3, 1);
        double A2312 = m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2);
        double A1312 = m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1);
        double A1212 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);
        double A0313 = m(1, 0) * m(3, 3) - m(1, 3) * m(3, 0);
        double A0213 = m(1, 0) * m(3, 2) - m(1, 2) * m(3, 0);
        double A0312 = m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0);
        double A0212 = m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0);
        double A0113 = m(1, 0) * m(3, 1) - m(1, 1) * m(3, 0);
        double A0112 = m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0);

        double det = m(0, 0) * (m(1, 1) * A2323 - m(1, 2) * A1323 + m(1, 3) * A1223)
            - m(0, 1) * (m(1, 0) * A2323 - m(1, 2) * A0323 + m(1, 3) * A0223)
            + m(0, 2) * (m(1, 0) * A1323 - m(1, 1) * A0323 + m(1, 3) * A0123)
            - m(0, 3) * (m(1, 0) * A1223 - m(1, 1) * A0223 + m(1, 2) * A0123);
        if (Math::isZero(det)) {
            if (nullptr != succeed)
                *succeed = false;
            return Matrix4x4();
        }
        det = 1.0 / det;
        
        Matrix4x4 inverse({
            det *   (m(1, 1) * A2323 - m(1, 2) * A1323 + m(1, 3) * A1223),
            det * - (m(0, 1) * A2323 - m(0, 2) * A1323 + m(0, 3) * A1223),
            det *   (m(0, 1) * A2313 - m(0, 2) * A1313 + m(0, 3) * A1213),
            det * - (m(0, 1) * A2312 - m(0, 2) * A1312 + m(0, 3) * A1212),
            det * - (m(1, 0) * A2323 - m(1, 2) * A0323 + m(1, 3) * A0223),
            det *   (m(0, 0) * A2323 - m(0, 2) * A0323 + m(0, 3) * A0223),
            det * - (m(0, 0) * A2313 - m(0, 2) * A0313 + m(0, 3) * A0213),
            det *   (m(0, 0) * A2312 - m(0, 2) * A0312 + m(0, 3) * A0212),
            det *   (m(1, 0) * A1323 - m(1, 1) * A0323 + m(1, 3) * A0123),
            det * - (m(0, 0) * A1323 - m(0, 1) * A0323 + m(0, 3) * A0123),
            det *   (m(0, 0) * A1313 - m(0, 1) * A0313 + m(0, 3) * A0113),
            det * - (m(0, 0) * A1312 - m(0, 1) * A0312 + m(0, 3) * A0112),
            det * - (m(1, 0) * A1223 - m(1, 1) * A0223 + m(1, 2) * A0123),
            det *   (m(0, 0) * A1223 - m(0, 1) * A0223 + m(0, 2) * A0123),
            det * - (m(0, 0) * A1213 - m(0, 1) * A0213 + m(0, 2) * A0113),
            det *   (m(0, 0) * A1212 - m(0, 1) * A0212 + m(0, 2) * A0112)
        });
        
        if (nullptr != succeed)
            *succeed = true;
        
        return inverse;
    }
    
    inline Matrix4x4 &operator*=(const Matrix4x4 &other)
    {
        Matrix4x4 m = (*this) * other;
        memcpy(m_data, m.constData(), sizeof(m_data));
        return *this;
    }
    
    Matrix4x4 &operator=(const Matrix4x4 &other)
    {
        if (this == &other)
            return *this;
     
        memcpy(m_data, other.constData(), sizeof(m_data));
        return *this;
    }
    
    Vector3 transformedPosition(const Vector3 &v) const
    {
        std::array<double, 4> vector4({v[0], v[1], v[2], 1.0});
        vector4 = (*this) * vector4;
        if (!Math::isZero(vector4[3])) {
            vector4[3] = 1.0 / vector4[3];
            vector4[0] *= vector4[3];
            vector4[1] *= vector4[3];
            vector4[2] *= vector4[3];
        }
        return Vector3 {vector4[0], vector4[1], vector4[2]};
    }

    void toTranslationAndRotation(Vector3 &translation, Quaternion &rotation) const
    {        
        translation = Vector3((*this)(3, 0), (*this)(3, 1), (*this)(3, 2));
        
        double scalar = std::sqrt(std::max(0.0, 1.0 + (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2))) / 2.0;
        double x = std::sqrt(std::max(0.0, 1.0 + (*this)(0, 0) - (*this)(1, 1) - (*this)(2, 2))) / 2.0;
        double y = std::sqrt(std::max(0.0, 1.0 - (*this)(0, 0) + (*this)(1, 1) - (*this)(2, 2))) / 2.0;
        double z = std::sqrt(std::max(0.0, 1.0 - (*this)(0, 0) - (*this)(1, 1) + (*this)(2, 2))) / 2.0;
        x *= x * ((*this)(1, 2) - (*this)(2, 1)) > 0 ? 1 : -1;
        y *= y * ((*this)(2, 0) - (*this)(0, 2)) > 0 ? 1 : -1;
        z *= z * ((*this)(0, 1) - (*this)(1, 0)) > 0 ? 1 : -1;
        double length = std::sqrt(scalar * scalar + x * x + y * y + z * z);
        rotation = Quaternion(scalar / length, x / length, y / length, z / length);
    }

private:
    double m_data[16] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
};

inline std::array<double, 4> operator*(const Matrix4x4 &m, const std::array<double, 4> &v)
{
    const double *data = m.constData();
    return std::array<double, 4>({
        (v[0] * data[0]) + (v[1] * data[4]) + (v[2] * data[8]) + (v[3] * data[12]),
        (v[0] * data[1]) + (v[1] * data[5]) + (v[2] * data[9]) + (v[3] * data[13]),
        (v[0] * data[2]) + (v[1] * data[6]) + (v[2] * data[10]) + (v[3] * data[14]),
        (v[0] * data[3]) + (v[1] * data[7]) + (v[2] * data[11]) + (v[3] * data[15])
    });
}

inline Vector3 operator*(const Matrix4x4 &m, const Vector3 &v)
{
    std::array<double, 4> vector4({v[0], v[1], v[2], 0.0});
    vector4 = m * vector4;
    return Vector3 {vector4[0], vector4[1], vector4[2]};
}

inline Matrix4x4 operator*(const Matrix4x4 &a, const Matrix4x4 &b)
{
    Matrix4x4 m;
    double *dest = m.data();
    const double *dataA = a.constData();
    const double *dataB = b.constData();
    
    dest[0] = dataA[0] * dataB[0] +
        dataA[4] * dataB[1] +
        dataA[8] * dataB[2] +
        dataA[12] * dataB[3];
    dest[4] = dataA[0] * dataB[4] +
        dataA[4] * dataB[5] +
        dataA[8] * dataB[6] +
        dataA[12] * dataB[7];
    dest[8] = dataA[0] * dataB[8] +
        dataA[4] * dataB[9] +
        dataA[8] * dataB[10] +
        dataA[12] * dataB[11];
    dest[12] = dataA[0] * dataB[12] +
        dataA[4] * dataB[13] +
        dataA[8] * dataB[14] +
        dataA[12] * dataB[15];

    dest[1] = dataA[1] * dataB[0] +
        dataA[5] * dataB[1] +
        dataA[9] * dataB[2] +
        dataA[13] * dataB[3];
    dest[5] = dataA[1] * dataB[4] +
        dataA[5] * dataB[5] +
        dataA[9] * dataB[6] +
        dataA[13] * dataB[7];
    dest[9] = dataA[1] * dataB[8] +
        dataA[5] * dataB[9] +
        dataA[9] * dataB[10] +
        dataA[13] * dataB[11];
    dest[13] = dataA[1] * dataB[12] +
        dataA[5] * dataB[13] +
        dataA[9] * dataB[14] +
        dataA[13] * dataB[15];

    dest[2] = dataA[2] * dataB[0] +
        dataA[6] * dataB[1] +
        dataA[10] * dataB[2] +
        dataA[14] * dataB[3];
    dest[6] = dataA[2] * dataB[4] +
        dataA[6] * dataB[5] +
        dataA[10] * dataB[6] +
        dataA[14] * dataB[7];
    dest[10] = dataA[2] * dataB[8] +
        dataA[6] * dataB[9] +
        dataA[10] * dataB[10] +
        dataA[14] * dataB[11];
    dest[14] = dataA[2] * dataB[12] +
        dataA[6] * dataB[13] +
        dataA[10] * dataB[14] +
        dataA[14] * dataB[15];

    dest[3] = dataA[3] * dataB[0] +
        dataA[7] * dataB[1] +
        dataA[11] * dataB[2] +
        dataA[15] * dataB[3];
    dest[7] = dataA[3] * dataB[4] +
        dataA[7] * dataB[5] +
        dataA[11] * dataB[6] +
        dataA[15] * dataB[7];
    dest[11] = dataA[3] * dataB[8] +
        dataA[7] * dataB[9] +
        dataA[11] * dataB[10] +
        dataA[15] * dataB[11];
    dest[15] = dataA[3] * dataB[12] +
        dataA[7] * dataB[13] +
        dataA[11] * dataB[14] +
        dataA[15] * dataB[15];
        
    return m;
}

}

#endif
