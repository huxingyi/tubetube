R"################(#version 300 es

precision highp float;
uniform vec4 id;
out vec4 fragColor;
void main()
{
    fragColor = id;
}

)################"