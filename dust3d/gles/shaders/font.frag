R"################(#version 300 es

precision lowp float;
uniform sampler2D fontMap;
uniform vec4 objectColor;
in vec2 pointTexCoords;
out vec4 fragColor;
void main()
{
    fragColor = objectColor * vec4(1.0, 1.0, 1.0, texture(fontMap, pointTexCoords).r);
}

)################"