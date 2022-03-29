R"################(#version 300 es

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float objectId;
layout(location = 0) in vec4 vertexPosition;
out vec4 pointPosition;
void main()
{
    pointPosition = modelMatrix * vertexPosition;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;
}

)################"