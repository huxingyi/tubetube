R"################(#version 300 es

precision lowp float;
uniform vec4 objectColor;
out vec4 fragColor;
void main()
{
    fragColor = objectColor;
}

)################"