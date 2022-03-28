R"################(#version 300 es

precision mediump float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D debugMap;

void main()
{
    vec3 color = texture(debugMap, pointTexCoords).rgb;
    fragColor = vec4(color, 1.0);
}

)################"
