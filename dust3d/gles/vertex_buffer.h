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

#ifndef DUST3D_GLES_VERTEX_BUFFER_H_
#define DUST3D_GLES_VERTEX_BUFFER_H_

#include <vector>
#include <GLES2/gl2.h>

namespace dust3d
{

class VertexBuffer
{
public:
    VertexBuffer() = default;
    
    VertexBuffer(std::unique_ptr<std::vector<GLfloat>> vertices, size_t numbersPerVertex, size_t vertexCount, uint32_t drawHint=0)
    {
        update(std::move(vertices), numbersPerVertex, vertexCount, drawHint);
    }
    
    bool begin()
    {
        if (nullptr != m_vertices) {
            release();
        }
        if (0 == m_vertexBufferId) {
            glGenBuffers(1, &m_vertexBufferId);
            glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_numbersPerVertex * m_vertexCount, m_vertices->data(), GL_STATIC_DRAW);
            delete m_vertices.release();
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
        return true;
    }
    
    void end()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void update(std::unique_ptr<std::vector<GLfloat>> vertices, size_t numbersPerVertex, size_t vertexCount, uint32_t drawHint)
    {
        m_vertices = std::move(vertices);
        m_numbersPerVertex = numbersPerVertex;
        m_vertexCount = vertexCount;
        m_drawHint = drawHint;
    }
    
    size_t numbersPerVertex() const
    {
        return m_numbersPerVertex;
    }
    
    size_t vertexCount() const
    {
        return m_vertexCount;
    }
    
    void release()
    {
        glDeleteBuffers(1, &m_vertexBufferId);
        m_vertexBufferId = 0;
    }
    
    uint32_t drawHint() const
    {
        return m_drawHint;
    }
    
private:
    GLuint m_vertexBufferId = 0;
    size_t m_vertexCount = 0;
    size_t m_numbersPerVertex = 0;
    uint32_t m_drawHint = 0;
    std::unique_ptr<std::vector<GLfloat>> m_vertices;
};

}

#endif
