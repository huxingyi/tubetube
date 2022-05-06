R"################(#version 300 es

layout(location = 0) in vec2 vertexPosition;
uniform mat4 projectionMatrix;
out vec2 pointPosition;
void main()
{
    pointPosition = vertexPosition.xy;
    gl_Position = projectionMatrix * vec4(vertexPosition.xy, 0.0, 1.0);
}

)################"