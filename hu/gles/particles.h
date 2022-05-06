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

#ifndef HU_GLES_PARTICLES_H_
#define HU_GLES_PARTICLES_H_

#include <array>
#include <queue>
#include <hu/gles/shader.h>

namespace Hu
{

class Particles
{
public:
    static const size_t m_total = 4096;
    
    struct Element
    {
        float timeRangeAndRadius[3];
        float startPosition[3];
        float velocity[3];
        float startColor[3];
        float stopColor[3];
    };
    
    struct Node
    {
        size_t elementIndex;
        float endTime;
    };
    
    class CompareNode
    {
    public:
        bool operator() (const Node &first, const Node &second)
        {
            return first.endTime > second.endTime;
        }
    };

    Particles()
    {
        memset(&m_elements[0], 0, sizeof(m_elements));
        for (size_t i = 0; i < m_elements.size(); ++i)
            m_freeIndices.push(i);
    }
    
    void initialize()
    {
        const GLchar *vertexShaderSource =
            #include <hu/gles/shaders/particle.vert>
            ;
        const GLchar *fragmentShaderSource = 
            #include <hu/gles/shaders/particle.frag>
            ;
        m_shader = std::unique_ptr<Shader>(new Shader(vertexShaderSource, fragmentShaderSource));
    }
    
    void addElement(const Element &element)
    {
        if (m_freeIndices.empty())
            return;
        size_t elementIndex = m_freeIndices.front();
        m_freeIndices.pop();
        m_elements[elementIndex] = element;
        m_nodes.push({elementIndex, element.timeRangeAndRadius[1]});
    }
    
    size_t aliveElementCount() const
    {
        return m_nodes.size();
    }
    
    Shader &shader()
    {
        return *m_shader;
    }
    
    const std::array<Element, m_total> &elements() const
    {
        return m_elements;
    }
    
    void update(float time)
    {
        while (!m_nodes.empty() && m_nodes.top().endTime < time) {
            m_freeIndices.push(m_nodes.top().elementIndex);
            m_nodes.pop();
        }
    }

    std::array<Element, m_total> m_elements;
    std::priority_queue<Node, std::vector<Node>, CompareNode> m_nodes;
    std::unique_ptr<Shader> m_shader;
    std::queue<size_t> m_freeIndices;
};

}

#endif
