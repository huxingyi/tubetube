R"################(#version 300 es

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightViewProjectionMatrix;
layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
out vec4 pointNormal;
out vec4 pointPosition;
out vec4 shadowCoord;
void main()
{
    pointNormal = normalize(modelMatrix * vertexNormal);
    pointPosition = modelMatrix * vertexPosition;
    shadowCoord = (lightViewProjectionMatrix * modelMatrix * vertexPosition) * 0.5 + 0.5;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;
}

)################"