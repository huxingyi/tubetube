R"################(#version 300 es

uniform float time;
uniform vec3 centerPosition;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
layout(location = 0) in float vertexLifetime;
layout(location = 1) in vec3 vertexStartPosition;
layout(location = 2) in vec3 vertexEndPosition;
out float pointLifetime;
void main()
{
    if (time <= vertexLifetime) {
        gl_Position.xyz = vertexStartPosition + (time * vertexEndPosition);
        gl_Position.xyz += centerPosition;
        gl_Position.w = 1.0;
        gl_Position = projectionMatrix * viewMatrix * gl_Position;
    } else {
        gl_Position = vec4(-1000, -1000, 0, 0);
    }
    pointLifetime = 1.0 - (time / vertexLifetime);
    pointLifetime = clamp(pointLifetime, 0.0, 1.0);
    gl_PointSize = (pointLifetime * pointLifetime) * 40.0;
}

)################"
