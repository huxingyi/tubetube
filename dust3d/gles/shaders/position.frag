R"################(#version 300 es

precision lowp float;
out vec4 fragColor;
in vec4 pointPosition;
void main()
{
    fragColor = pointPosition;
}

)################"