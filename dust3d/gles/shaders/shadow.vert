R"################(#version 300 es

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
layout(location = 0) in vec4 vertexPosition;
out vec4 pointColor;
void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;
}

)################"