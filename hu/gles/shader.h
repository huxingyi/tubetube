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

#ifndef HU_GLES_SHADER_H_
#define HU_GLES_SHADER_H_

#include <string>
#include <map>
#include <GLES2/gl2.h>
#include <hu/base/color.h>
#include <hu/base/matrix4x4.h>

namespace Hu
{

class Shader
{
public:
    Shader()
    {
    }
    
    Shader(const char *vertexShaderSource, const char *fragmentShaderSource)
    {
        GLuint vertexShader(glCreateShader(GL_VERTEX_SHADER));
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        checkCompileError(vertexShader);
        
        GLuint fragmentShader(glCreateShader(GL_FRAGMENT_SHADER));
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        checkCompileError(fragmentShader);
        
        m_program = glCreateProgram();
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);
        checkLinkError(m_program);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    void use()
    {
        glUseProgram(m_program);
    }

    GLuint getUniformLocation(const std::string &name)
    {
        auto findLocation = m_uniformLocationMap.find(name);
        if (findLocation != m_uniformLocationMap.end()) {
            return findLocation->second;
        }
        GLuint location = glGetUniformLocation(m_program, name.c_str());
        m_uniformLocationMap.insert({name, location});
        return location;
    }
    
    void setUniformMatrix(const std::string &name, const Matrix4x4 &matrix)
    {
        GLfloat matrixData[16];
        matrix.getData(matrixData);
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrixData[0]);
    }
    
    void setUniformColor(const std::string &name, const Color &color)
    {
        glUniform4f(getUniformLocation(name), color[0], color[1], color[2], color[3]);
    }
    
private:
    GLuint m_program = 0;
    std::map<std::string, GLuint> m_uniformLocationMap;
    
    void checkCompileError(GLuint shader)
    {
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            GLchar *strInfoLog = new GLchar[logLength + 1];
            glGetShaderInfoLog(shader, logLength, NULL, strInfoLog);
            std::cerr << "Compile error log:\n" << strInfoLog << "\n";
            delete[] strInfoLog;
        }
    }
    
    void checkLinkError(GLuint program)
    {
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            GLchar *strInfoLog = new GLchar[logLength + 1];
            glGetProgramInfoLog(program, logLength, NULL, strInfoLog);
            std::cerr << "Link error log:\n" << strInfoLog << "\n";
            delete[] strInfoLog;
        }
    }
};

}

#endif
