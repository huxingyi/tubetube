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

#ifndef DUST3D_MESH_TUBE_MESH_BUILDER_H_
#define DUST3D_MESH_TUBE_MESH_BUILDER_H_

#include <memory>
#include <algorithm>
#include <set>
#include <dust3d/base/debug.h>
#include <dust3d/base/vector2.h>

namespace dust3d
{
    
class TubeMeshBuilder
{
public:
    enum SectionFillPattern
    {
        None,
        Strips
    };

    struct Section
    {
        Vector3 origin;
        double radius;
        Vector3 normal;
        Vector3 bitangent;
        std::vector<Vector2> polygon;
        std::vector<int> profilePoints;
    };
    
    TubeMeshBuilder(std::unique_ptr<std::vector<Section>> sections)
    {
        m_sections = std::move(sections);
    }
    
    void setSectionFillPattern(SectionFillPattern pattern)
    {
        m_sectionFillPattern = pattern;
    }
    
    bool build()
    {
        if (nullptr == m_sections)
            return false;
        
        std::vector<std::vector<Vector3>> sectionPolygons(m_sections->size());
        
        for (size_t i = 0; i < m_sections->size(); ++i) {
            if (!makeSectionPolygon(m_sections->at(i), sectionPolygons[i])) {
                dust3dDebug << "Make section polygon failed on:[" << i << "/" << m_sections->size() << "].";
                return false;
            }
        }
        
        std::vector<std::vector<size_t>> meshVertexIndices(sectionPolygons.size());
        m_meshVertices = std::make_unique<std::vector<Vector3>>();
        for (size_t i = 0; i < sectionPolygons.size(); ++i) {
            meshVertexIndices[i].resize(sectionPolygons[i].size());
            for (size_t j = 0; j < sectionPolygons[i].size(); ++j) {
                meshVertexIndices[i][j] = m_meshVertices->size();
                m_meshVertices->push_back(sectionPolygons[i][j]);
            }
        }
        
        m_meshProfileEdges = std::make_unique<std::set<std::pair<size_t, size_t>>>();
        
        m_meshQuads = std::make_unique<std::vector<std::vector<size_t>>>();
        for (size_t j = 1; j < sectionPolygons.size(); ++j) {
            size_t i = j - 1;
            const auto &edgeLoopI = meshVertexIndices[i];
            const auto &edgeLoopJ = meshVertexIndices[j];
            if (edgeLoopI.size() != edgeLoopJ.size()) {
                dust3dDebug << "Edge loop have unmatched size [" << i << "]:" << edgeLoopI.size() << "[" << j << "]:" << edgeLoopJ.size() << ".";
                return false;
            }
            for (const auto &it: m_sections->at(i).profilePoints)
                m_meshProfileEdges->insert({edgeLoopI[it % edgeLoopI.size()], edgeLoopJ[it % edgeLoopJ.size()]});
            for (size_t m = 0; m < edgeLoopI.size(); ++m) {
                size_t n = (m + 1) % edgeLoopI.size();
                m_meshQuads->push_back({
                    edgeLoopI[m], 
                    edgeLoopI[n],
                    edgeLoopJ[n],
                    edgeLoopJ[m]
                });
            }
        }
        
        if (meshVertexIndices.size() > 1) {
            auto indices = meshVertexIndices.front();
            std::reverse(indices.begin(), indices.end());
            fillSection(indices);
            for (size_t i = 0; i < meshVertexIndices.front().size(); ++i)
                m_meshProfileEdges->insert({meshVertexIndices.front()[i], meshVertexIndices.front()[(i + 1) % meshVertexIndices.front().size()]});
        }
        fillSection(meshVertexIndices.back());
        for (size_t i = 0; i < meshVertexIndices.back().size(); ++i)
            m_meshProfileEdges->insert({meshVertexIndices.back()[i], meshVertexIndices.back()[(i + 1) % meshVertexIndices.back().size()]});
        
        return true;
    }
    
    std::unique_ptr<std::vector<Vector3>> takeMeshVertices()
    {
        return std::move(m_meshVertices);
    }
    
    std::unique_ptr<std::vector<std::vector<size_t>>> takeMeshQuads()
    {
        return std::move(m_meshQuads);
    }
    
    std::unique_ptr<std::set<std::pair<size_t, size_t>>> takeMeshProfileEdges()
    {
        return std::move(m_meshProfileEdges);
    }
    
    void getMeshTriangles(std::vector<std::vector<size_t>> &triangles)
    {
        if (nullptr == m_meshQuads)
            return;
        triangles.resize(m_meshQuads->size() * 2);
        for (size_t i = 0, targetIndex = 0; i < m_meshQuads->size(); ++i) {
            const auto &quad = m_meshQuads->at(i);
            triangles[targetIndex++] = std::vector<size_t> {quad[0], quad[1], quad[2]};
            triangles[targetIndex++] = std::vector<size_t> {quad[2], quad[3], quad[0]};
        }
    }
    
private:
    SectionFillPattern m_sectionFillPattern = SectionFillPattern::Strips;
    std::unique_ptr<std::vector<Section>> m_sections;
    std::unique_ptr<std::vector<Vector3>> m_meshVertices;
    std::unique_ptr<std::vector<std::vector<size_t>>> m_meshQuads;
    std::unique_ptr<std::set<std::pair<size_t, size_t>>> m_meshProfileEdges;
    
    bool fillSection(const std::vector<size_t> &polygon)
    {
        switch (m_sectionFillPattern) {
            case SectionFillPattern::Strips: {
                    if (0 != polygon.size() % 2) {
                        dust3dDebug << "Polygon could not be fill strips with odd size:" << polygon.size();
                        return false;
                    }
                    size_t halfSize = polygon.size() / 2;
                    size_t quartSize = halfSize / 2;
                    // FIXME: Not sure if it will work on both odd/even case
                    for (size_t i = 0; i < quartSize; i += 2) {
                        m_meshQuads->push_back({
                            polygon[i],
                            polygon[i + 1],
                            polygon[(i + halfSize - 1) % polygon.size()],
                            polygon[(i + halfSize) % polygon.size()]
                        });
                    }
                    for (size_t i = halfSize; i < halfSize + quartSize; i += 2) {
                        m_meshQuads->push_back({
                            polygon[i],
                            polygon[i + 1],
                            polygon[(i + halfSize - 1) % polygon.size()],
                            polygon[(i + halfSize) % polygon.size()]
                        });
                    }
                } break;
        }
        return true;
    }

    bool makeSectionPolygon(const Section &section, std::vector<Vector3> &polygon3d)
    {
        if (section.polygon.size() <= 2) {
            dust3dDebug << "Polygon requires three points at least, current points:" << section.polygon.size();
            return false;
        }
        
        std::vector<double> radiusList(section.polygon.size());
        for (size_t i = 0; i < section.polygon.size(); ++i)
            radiusList[i] = section.polygon[i].length();
        double maxRadius = *std::max_element(radiusList.begin(), radiusList.end(), [](const auto &first, const auto &second) {
            return first < second;
        });
        if (Math::isZero(maxRadius)) {
            dust3dDebug << "Polygon max radius is zero";
            return false;
        }
        for (auto &it: radiusList)
            it /= maxRadius;
        
        polygon3d.resize(section.polygon.size());
        for (size_t i = 0; i < section.polygon.size(); ++i) {
            double angle2d = Vector3::angle(
                Vector3(0.0, 1.0, 0.0), // Up
                Vector3(section.polygon[i]), 
                Vector3(0.0, 0.0, 1.0) // Out
            );
            auto tangent = Vector3::crossProduct(section.bitangent, section.normal);
            auto recalculatedBitangent = Vector3::crossProduct(section.normal, tangent).normalized();
            polygon3d[i] = section.origin + 
                recalculatedBitangent.rotated(section.normal, angle2d) * radiusList[i] * section.radius;
        }
        
        return true;
    }
};
    
};

#endif

