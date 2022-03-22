R"################(#version 300 es

precision lowp float;
uniform vec4 particleColor;
uniform sampler2D particleMap;
in float pointLifetime;
out vec4 fragColor;
void main()
{
    vec4 texColor;
    texColor = texture(particleMap, gl_PointCoord);
    fragColor = vec4(particleColor) * texColor;
    fragColor.a *= pointLifetime;
}

)################"
