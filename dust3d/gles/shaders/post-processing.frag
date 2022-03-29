R"################(#version 300 es

precision highp float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D colorMap;
uniform sampler2D uiMap;
uniform sampler2D depthMap;
uniform sampler2D positionMap;

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

float depthRange()
{
    ivec2 imageSize = textureSize(positionMap, 0).xy;
    float imageWidth = float(imageSize.x);
    float imageHeight = float(imageSize.y);
    
    float depth = texture(depthMap, pointTexCoords).r * 0.5 + 0.5;
    
    const float checkSize = 1.0;
    float maxDistance = 0.0;
    vec4 currentPosition = texture(positionMap, pointTexCoords).rgba;
    float x;
    float y = +1.0;
    for (x = -checkSize; x <= checkSize; x += 1.0) {
        vec2 lookupAt = vec2((float(gl_FragCoord.x) + x) / imageWidth, (float(gl_FragCoord.y) + y) / imageHeight);
        vec4 checkPosition = texture(positionMap, lookupAt).rgba;
        // float(checkPosition.y > 0.0 || currentPosition.y > 0.0) Is to avoid large model glitches around axis lines
        maxDistance = max(maxDistance, distance(checkPosition.rgb, currentPosition.rgb) * float(checkPosition.y > 0.0 || currentPosition.y > 0.0));
    }
    return float(depth < 0.9999) * maxDistance;  // Use depth < 0.9999 to avoid horizontal line been outlined
}

void main()
{
    vec3 color = texture(colorMap, pointTexCoords).rgb;
    vec3 blurColor = blur();
    float depth = texture(depthMap, pointTexCoords).r * 0.5 + 0.5;
    color = mix(color, blurColor, smoothstep(0.995, 1.0, depth));
    vec4 uiColor = texture(uiMap, pointTexCoords).rgba;
    color = uiColor.rgb * uiColor.a + color.rgb * (1.0 - uiColor.a);
    //fragColor = vec4(vec3(depthRange()), 1.0);
    fragColor = vec4(mix(color, vec3(0.45, 0.31, 0.15), depthRange()), 1.0);
    //fragColor = texture(positionMap, pointTexCoords);
}

)################"
