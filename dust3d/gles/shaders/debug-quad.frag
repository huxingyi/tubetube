R"################(#version 300 es

precision mediump float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D debugMap;

void main()
{
    float depthValue = texture(debugMap, pointTexCoords).r;
    fragColor = vec4(vec3(depthValue), 1.0);
}

)################"
