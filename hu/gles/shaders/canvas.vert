R"################(#version 300 es

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec4 vertexColor;
uniform mat4 projectionMatrix;
out vec4 pointColor;
void main()
{
    pointColor = vertexColor;
    gl_Position = projectionMatrix * vec4(vertexPosition.xy, 0.0, 1.0);
}

)################"