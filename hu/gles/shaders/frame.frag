R"################(#version 300 es

precision highp float;
uniform vec4 objectColor;
uniform vec4 frameCoords;
uniform float frameCornerRadius;
in vec2 pointPosition;
out vec4 fragColor;
void main()
{
    if (length(pointPosition - vec2(frameCoords[0] + frameCornerRadius, frameCoords[1] + frameCornerRadius)) <= frameCornerRadius ||
            length(pointPosition - vec2(frameCoords[2] - frameCornerRadius, frameCoords[1] + frameCornerRadius)) <= frameCornerRadius ||
            length(pointPosition - vec2(frameCoords[2] - frameCornerRadius, frameCoords[3] - frameCornerRadius)) <= frameCornerRadius ||
            length(pointPosition - vec2(frameCoords[0] + frameCornerRadius, frameCoords[3] - frameCornerRadius)) <= frameCornerRadius ||
            pointPosition[0] >= frameCoords[0] + frameCornerRadius && pointPosition[0] <= frameCoords[2] - frameCornerRadius ||
            pointPosition[1] >= frameCoords[1] + frameCornerRadius && pointPosition[1] <= frameCoords[3] - frameCornerRadius)
        fragColor = objectColor;
    else
        discard;
}

)################"