R"################(#version 300 es

precision highp float;
uniform sampler2D imageMap;
in vec2 pointTexCoords;
out vec4 fragColor;
void main()
{
    fragColor = texture(imageMap, pointTexCoords).rgba;
}

)################"