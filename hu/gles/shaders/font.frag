R"################(#version 300 es

precision highp float;
uniform sampler2D fontMap;
uniform vec4 objectColor;
in vec2 pointTexCoords;
out vec4 fragColor;

const float pxRange = 4.0;

float median(float r, float g, float b) 
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() 
{
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(fontMap, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(pointTexCoords);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main()
{
    vec3 msd = texture(fontMap, pointTexCoords).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    
    fragColor = objectColor * vec4(1.0, 1.0, 1.0, alpha);
}

)################"