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
    // 3x3 kernel with 4 taps per sample, effectively 6x6 PCF
    float sum = 0.0;
    float x, y;
    for (x = -2.0; x <= 2.0; x += 2.0)
        for (y = -2.0; y <= 2.0; y += 2.0)
            sum += shadowLookup(x, y);

    // divide sum by 9.0
    return sum * 0.11;
}

void main()
{
    vec3 resultColor;
    resultColor += calculateDirectionLight(directionLight);
    for (int i = 0; i < POINT_LIGHT_COUNT; i++)
        resultColor += calculatePointLight(pointLights[i]);
    fragColor = vec4(gammaCorrection(resultColor * shadow()), 1.0);
}

)################"