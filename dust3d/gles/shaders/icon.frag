R"################(#version 300 es

precision highp float;
uniform sampler2D iconMap;
uniform vec4 objectColor;
in vec2 pointTexCoords;
out vec4 fragColor;
void main()
{
    float alpha = texture(iconMap, pointTexCoords).r;
    fragColor = objectColor * vec4(1.0, 1.0, 1.0, alpha);
}

)################"