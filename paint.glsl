#version 330

in vec2 UV;

out vec4 color;

void main()
{
    vec4 colorA = vec4(0.8, 0.4, 0.2, 1.0);
    vec4 colorB = vec4(0.1, 0.6, 0.5, 1.0);
    vec4 colorC = vec4(0.3, 0.2, 0.7, 1.0);
    vec4 colorD = vec4(0.5, 0.8, 0.9, 1.0);

    color = UV.x * UV.y * colorA
            + UV.x * (1-UV.y) * colorB
            + (1-UV.x) * UV.y * colorC
            + (1-UV.x) * (1-UV.y) * colorD;
}
