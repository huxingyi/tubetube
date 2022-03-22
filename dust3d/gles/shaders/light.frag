R"################(#version 300 es

precision mediump float;
uniform vec4 lightColor;
in vec4 pointNormal;
in vec4 pointPosition;
out vec4 fragColor;
void main()
{
    fragColor = lightColor;
}

)################"