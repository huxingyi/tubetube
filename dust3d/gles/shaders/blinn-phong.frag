R"################(#version 300 es

precision lowp float;
struct PointLight
{
    vec4 position;
    vec4 color;
    float constant;
    float linear;
    float quadratic;
    float ambient;
    float diffuse;
    float specular;
};
#define POINT_LIGHT_COUNT         2
uniform PointLight pointLights[POINT_LIGHT_COUNT];
struct DirectionLight
{
    vec4 direction;
    vec4 color;
    float ambient;
    float diffuse;
    float specular;
};
uniform DirectionLight directionLight;
uniform vec4 objectColor;
uniform vec4 cameraPosition;
uniform lowp sampler2DShadow shadowMap;
in vec4 pointNormal;
in vec4 pointPosition;
in vec4 pointColor;
in vec4 shadowCoord;
out vec4 fragColor;

const float gamma = 2.2;

vec3 gammaCorrection(vec3 color)
{
    return pow(color, vec3(1.0 / gamma));
}

vec3 calculateDirectionLight(DirectionLight light)
{
    vec3 lightDirection = -light.direction.xyz;
    
    vec3 ambientColor = light.ambient * light.color.rgb;
    
    float diffuse = max(dot(pointNormal.xyz, lightDirection.xyz), 0.0);
    vec3 diffuseColor = light.diffuse * diffuse * light.color.rgb;
    
    float shininess = 8.0;
    vec3 viewDirection = normalize(cameraPosition.xyz - pointPosition.xyz);
    vec3 halfwayDirection = normalize(lightDirection.xyz + viewDirection.xyz);
    float specular = pow(max(dot(pointNormal.xyz, halfwayDirection), 0.0), shininess);
    vec3 specularColor = light.specular * specular * light.color.rgb;
    
    return (ambientColor + diffuseColor + specularColor) * pointColor.rgb;
}

vec3 calculatePointLight(PointLight light)
{
    vec3 ambientColor = light.ambient * light.color.rgb;
    
    vec3 lightDirection = normalize(light.position.xyz - pointPosition.xyz);
    float lightDistance = length(light.position.xyz - pointPosition.xyz);
    float lightAttenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance));
    
    float diffuse = max(dot(pointNormal.xyz, lightDirection.xyz), 0.0);
    vec3 diffuseColor = light.diffuse * diffuse * light.color.rgb;
    
    float shininess = 32.0;
    vec3 viewDirection = normalize(cameraPosition.xyz - pointPosition.xyz);
    vec3 halfwayDirection = normalize(lightDirection.xyz + viewDirection.xyz);
    float specular = pow(max(dot(pointNormal.xyz, halfwayDirection), 0.0), shininess);
    vec3 specularColor = light.specular * specular * light.color.rgb;
    
    return (ambientColor + diffuseColor + specularColor) * lightAttenuation * pointColor.rgb;
}

float shadowLookup(float x, float y)
{
   float pixelSize = 0.002;
   vec4 offset = vec4(x * pixelSize * shadowCoord.w,
                      y * pixelSize * shadowCoord.w,
                      -0.005 * shadowCoord.w, 
                      0.0);
   return textureProj(shadowMap, shadowCoord + offset);
}

float shadow()
{
    float sum = 0.0;
    float num = 0.0;
    float x, y;
    for (x = -2.0; x <= 2.0; x += 2.0) {
        for (y = -2.0; y <= 2.0; y += 2.0) {
            sum += shadowLookup(x, y);
            num += 1.0;
        }
    }
    return sum / num;
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
    /*
    vec3 resultColor;
    resultColor += calculateDirectionLight(directionLight);
    for (int i = 0; i < POINT_LIGHT_COUNT; i++)
        resultColor += calculatePointLight(pointLights[i]);
    fragColor = vec4(gammaCorrection(resultColor * shadow()), 1.0);
    */
    vec3 hsl = rgbToHsl(pointColor.rgb);
    hsl.z += shadow() > 0.4 ? 0.1 : 0.0;
    fragColor = vec4(hslToRgb(hsl), 1.0);
}

)################"