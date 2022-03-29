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

#ifndef DUST3D_MESH_MESH_UTILS_H_
#define DUST3D_MESH_MESH_UTILS_H_

namespace dust3d
{

class MeshUtils
{
public:
    static void smoothNormal(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        const std::vector<Vector3> &triangleNormals,
        double thresholdAngle,
        std::vector<Vector3> &triangleVertexNormals)
    {
        std::vector<std::vector<std::pair<size_t, size_t>>> triangleVertexNormalsMapByIndices(vertices.size());
        std::vector<Vector3> angleAreaWeightedNormals;
        for (size_t triangleIndex = 0; triangleIndex < triangles.size(); ++triangleIndex) {
            const auto &sourceTriangle = triangles[triangleIndex];
            if (sourceTriangle.size() != 3) {
                continue;
            }
            const auto &v1 = vertices[sourceTriangle[0]];
            const auto &v2 = vertices[sourceTriangle[1]];
            const auto &v3 = vertices[sourceTriangle[2]];
            double area = Vector3::area(v1, v2, v3);
            double angles[] = {Vector3::angle(v2-v1, v3-v1),
                Vector3::angle(v1-v2, v3-v2),
                Vector3::angle(v1-v3, v2-v3)};
            for (int i = 0; i < 3; ++i) {
                if (sourceTriangle[i] >= vertices.size())
                    continue;
                triangleVertexNormalsMapByIndices[sourceTriangle[i]].push_back({triangleIndex, angleAreaWeightedNormals.size()});
                angleAreaWeightedNormals.push_back(triangleNormals[triangleIndex] * area * angles[i]);
            }
        }
        triangleVertexNormals = angleAreaWeightedNormals;
        std::map<std::pair<size_t, size_t>, double> degreesBetweenFacesMap;
        for (size_t vertexIndex = 0; vertexIndex < vertices.size(); ++vertexIndex) {
            const auto &triangleVertices = triangleVertexNormalsMapByIndices[vertexIndex];
            for (const auto &triangleVertex: triangleVertices) {
                for (const auto &otherTriangleVertex: triangleVertices) {
                    if (triangleVertex.first == otherTriangleVertex.first)
                        continue;
                    double angle = 0;
                    auto findDegreesResult = degreesBetweenFacesMap.find({triangleVertex.first, otherTriangleVertex.first});
                    if (findDegreesResult == degreesBetweenFacesMap.end()) {
                        angle = Vector3::angle(triangleNormals[triangleVertex.first], triangleNormals[otherTriangleVertex.first]);
                        degreesBetweenFacesMap.insert({{triangleVertex.first, otherTriangleVertex.first}, angle});
                        degreesBetweenFacesMap.insert({{otherTriangleVertex.first, triangleVertex.first}, angle});
                    } else {
                        angle = findDegreesResult->second;
                    }
                    if (angle > thresholdAngle)
                        continue;
                    triangleVertexNormals[triangleVertex.second] += angleAreaWeightedNormals[otherTriangleVertex.second];
                }
            }
        }
        for (auto &item: triangleVertexNormals)
            item.normalize();
    }

};
 
}

#endif

