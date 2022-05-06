R"################(#version 300 es

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexUv;
out vec2 pointTexCoords;
void main()
{
    pointTexCoords = vertexUv.xy;
    gl_Position = vertexPosition;
}

)################"