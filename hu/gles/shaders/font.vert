R"################(#version 300 es

layout(location = 0) in vec4 vertexPositionAndUv;
uniform mat4 projectionMatrix;
out vec2 pointTexCoords;
void main()
{
    pointTexCoords = vertexPositionAndUv.zw;
    gl_Position = projectionMatrix * vec4(vertexPositionAndUv.xy, 0.0, 1.0);
}

)################"