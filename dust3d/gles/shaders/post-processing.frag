R"################(#version 300 es

precision mediump float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D colorMap;
uniform sampler2D uiMap;
uniform sampler2D depthMap;

vec3 blur()
{
    ivec2 imageSize = textureSize(colorMap, 0).xy;
    float imageWidth = float(imageSize.x);
    float imageHeight = float(imageSize.y);
  
    const float blurSize = 2.0;
    vec3 sum = vec3(0.0);
    float num = 0.0;
    float x, y;
    for (x = -blurSize; x <= blurSize; x += 1.0) {
        for (y = -blurSize; y <= blurSize; y += 1.0) {
            sum += texture(colorMap, vec2((float(gl_FragCoord.x) + x) / imageWidth, (float(gl_FragCoord.y) + y) / imageHeight)).rgb;
            num += 1.0;
        }
    }
    return sum / num;
}

void main()
{
    vec3 color = texture(colorMap, pointTexCoords).rgb;
    vec3 blurColor = blur();
    float depth = texture(depthMap, pointTexCoords).r * 0.5 + 0.5;
    color = mix(color, blurColor, smoothstep(0.99, 1.0, depth));
    vec4 uiColor = texture(uiMap, pointTexCoords).rgba;
    color = uiColor.rgb * uiColor.a + color.rgb * (1.0 - uiColor.a);
    fragColor = vec4(color, 1.0);
}

)################"
