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

#ifndef DUST3D_GLES_VERTEX_BUFFER_UTILS_H_
#define DUST3D_GLES_VERTEX_BUFFER_UTILS_H_

#include <dust3d/gles/vertex_buffer.h>
#include <dust3d/base/vector3.h>

namespace dust3d
{
    
class VertexBufferUtils
{
public:
    static void loadTrangulatedMesh(VertexBuffer &vertexBuffer, 
        const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles)
    {
        auto vertexBufferVertices = std::make_unique<std::vector<GLfloat>>();
        size_t numbersPerVertex = 6;
        vertexBufferVertices->resize(triangles.size() * 3 * numbersPerVertex);
        for (size_t i = 0, targetIndex = 0; i < triangles.size(); ++i) {
            const auto &triangle = triangles[i];
            Vector3 triangleNormal = Vector3::normal(vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]]);
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[0]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[1]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)vertices[triangle[2]].z();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.x();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.y();
            vertexBufferVertices->at(targetIndex++) = (GLfloat)triangleNormal.z();
        }
        size_t vertexCount = vertexBufferVertices->size() / numbersPerVertex;
        vertexBuffer.update(std::move(vertexBufferVertices), numbersPerVertex, vertexCount);
    }
    
    static void loadMeshBorders(VertexBuffer &vertexBuffer, 
        const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &faces)
    {
        auto vertexBufferVertices = std::make_unique<std::vector<GLfloat>>();
        size_t numbersPerVertex = 3;
        for (size_t i = 0, targetIndex = 0; i < faces.size(); ++i) {
            const auto &face = faces[i];
            for (size_t m = 0; m < face.size(); ++m) {
                size_t n = (m + 1) % face.size();
                vertexBufferVertices->push_back((GLfloat)vertices[face[m]].x());
                vertexBufferVertices->push_back((GLfloat)vertices[face[m]].y());
                vertexBufferVertices->push_back((GLfloat)vertices[face[m]].z());
                vertexBufferVertices->push_back((GLfloat)vertices[face[n]].x());
                vertexBufferVertices->push_back((GLfloat)vertices[face[n]].y());
                vertexBufferVertices->push_back((GLfloat)vertices[face[n]].z());
            }
        }
        size_t vertexCount = vertexBufferVertices->size() / numbersPerVertex;
        vertexBuffer.update(std::move(vertexBufferVertices), numbersPerVertex, vertexCount);
    }
};
    
};

#endif


