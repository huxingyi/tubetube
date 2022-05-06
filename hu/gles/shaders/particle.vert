R"################(#version 300 es

uniform float time;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec2 windowSize;
layout(location = 0) in vec3 vertexTimeRangeAndRadius;
layout(location = 1) in vec3 vertexStartPosition;
layout(location = 2) in vec3 vertexVelocity;
layout(location = 3) in vec3 vertexStartColor;
layout(location = 4) in vec3 vertexStopColor;
out vec4 pointColor;
out vec2 pointCenter;
out float pointRadius;
void main()
{
    if (time <= vertexTimeRangeAndRadius.y) {
        gl_Position.xyz = vertexStartPosition + ((time - vertexTimeRangeAndRadius.x) * vertexVelocity);
        gl_Position.w = 1.0;
        gl_Position = projectionMatrix * viewMatrix * gl_Position;
    } else {
        gl_Position = vec4(-10000, -10000, 0, 1.0);
    }
    pointCenter = (0.5 * gl_Position.xy / gl_Position.w + 0.5) * windowSize;
    float pointAlpha = 1.0 - (time - vertexTimeRangeAndRadius.x) / (vertexTimeRangeAndRadius.y - vertexTimeRangeAndRadius.x);
    pointColor = vec4(mix(vertexStopColor, vertexStartColor, pointAlpha), pointAlpha);
    // https://gamedev.stackexchange.com/questions/54391/scaling-point-sprites-with-distance
    gl_PointSize = pointAlpha * windowSize.y * projectionMatrix[1][1] * vertexTimeRangeAndRadius.z / gl_Position.w;
    pointRadius = gl_PointSize / 2.0;
}

)################"
