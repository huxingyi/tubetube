R"################(#version 300 es

precision lowp float;
in vec4 pointColor;
in vec2 pointCenter;
in float pointRadius;
out vec4 fragColor;
void main()
{
    vec2 coord = (gl_FragCoord.xy - pointCenter) / pointRadius;
    float l = length(coord);
    if (l > 1.0)
        discard;
    fragColor = pointColor;
}

)################"
