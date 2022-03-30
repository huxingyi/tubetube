R"################(#version 300 es

precision highp float;
out vec4 fragColor;
in vec2 pointTexCoords;
uniform sampler2D colorMap;
uniform sampler2D uiMap;
uniform sampler2D depthMap;
uniform sampler2D positionMap;
uniform sampler2D idMap;
uniform float time;

const float groundId = 0.1;
const float waterId = 0.2;
const float epsilon = 0.01;
const float pi = 3.14159;

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

float depthDiff()
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

float foam()
{
    ivec2 imageSize = textureSize(idMap, 0).xy;
    float imageWidth = float(imageSize.x);
    float imageHeight = float(imageSize.y);
    const float duration = 10.0;
    float foamSize = 5.0;
    float sum = 0.0;
    float num = 0.0;
    float x;
    float y = +foamSize;
    for (x = -foamSize; x <= foamSize; x += 1.0) {
        vec2 lookupAt = vec2((float(gl_FragCoord.x) + x) / imageWidth, (float(gl_FragCoord.y) + y) / imageHeight);
        sum += float(abs(texture(idMap, lookupAt).r - groundId) <= epsilon);
        num += 1.0;
    }
    return float(abs(texture(idMap, pointTexCoords).r - waterId) <= epsilon) * sum / num;
}

vec3 rgbToHsl(vec3 color) 
{
    float maxChannel = max(max(color.r, color.g), color.b);
    float minChannel = min(min(color.r, color.g), color.b);
    float l = (maxChannel + minChannel) / 2.0;
    float h = l, s = l;
    if (maxChannel > minChannel) {
        float d = maxChannel - minChannel;
        s = l > 0.5 ? d / (2.0 - (maxChannel + minChannel)) : d / (maxChannel + minChannel);
        if (color.r >= maxChannel) {
            h = (color.g - color.b) / d + (color.g < color.b ? 6.0 : 0.0);
        } else if (color.g >= maxChannel) {
            h = (color.b - color.r) / d + 2.0;
        } else {
            h = (color.r - color.g) / d + 4.0;
        }
        h /= 6.0;
    } else {
        h = s = 0.0;
    }
    return vec3(h, s, l);
}

float hueToRgb(vec3 color)
{
    if (color.z < 0.0)
        color.z += 1.0;
    if (color.z > 1.0)
        color.z -= 1.0;
    if (color.z < 1.0 / 6.0)
        return color.x + (color.y - color.x) * 6.0 * color.z;
    if (color.z < 1.0 / 2.0)
        return color.y;
    if (color.z < 2.0 / 3.0)
        return color.x + (color.y - color.x) * (2.0 / 3.0 - color.z) * 6.0;
    return color.x;
}

vec3 hslToRgb(vec3 color) 
{
    float r, g, b;
    if (color.y > 0.0) {
        float q = color.z < 0.5 ? color.z * (1.0 + color.y) : color.z + color.y - color.z * color.y;
        float p = 2.0 * color.z - q;
        r = hueToRgb(vec3(p, q, color.x + 1.0 / 3.0));
        g = hueToRgb(vec3(p, q, color.x));
        b = hueToRgb(vec3(p, q, color.x - 1.0 / 3.0));
    } else {
        r = g = b = color.z;
    }
    return vec3(r, g, b);
}

void main()
{
    vec3 color = texture(colorMap, pointTexCoords).rgb;
    vec3 blurColor = blur();
    float depth = texture(depthMap, pointTexCoords).r * 0.5 + 0.5;
    depth = smoothstep(0.995, 1.0, depth);
    color = mix(color, blurColor, depth);
    vec4 uiColor = texture(uiMap, pointTexCoords).rgba;
    color = uiColor.rgb * uiColor.a + color.rgb * (1.0 - uiColor.a);
    vec3 foamHsl = rgbToHsl(color);
    foamHsl.z += foam() * 0.15;
    color = hslToRgb(foamHsl);
    float border = depthDiff();
    color = mix(color, vec3(0.45, 0.31, 0.15), border * (1.0 - depth));
    //fragColor = vec4(vec3(foam()), 1.0);
    fragColor = vec4(color, 1.0);
    //fragColor = texture(idMap, pointTexCoords);
}

)################"
