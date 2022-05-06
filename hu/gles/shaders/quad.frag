R"################(#version 300 es

precision mediump float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D colorMap;

void main()
{
    fragColor = texture(colorMap, pointTexCoords).rgba;
}

)################"
