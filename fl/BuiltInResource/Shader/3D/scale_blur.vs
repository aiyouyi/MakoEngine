#version 300 es
layout(location = 0) in  vec3 position;
uniform  float texelWidthOffset;
uniform  float texelHeightOffset;

out vec2 textureCoordinate;
out vec4 textureShift_1;
out vec4 textureShift_2;
out vec4 textureShift_3;
out vec4 textureShift_4;

void main()
{
    gl_Position = vec4(position,1.0);

    vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
    textureCoordinate = position.xy*0.5+0.5;
    textureShift_1 = vec4(textureCoordinate.xy - 2.0 * singleStepOffset, textureCoordinate.xy + 2.0 * singleStepOffset);
    textureShift_2 = vec4(textureCoordinate.xy - 4.0 * singleStepOffset, textureCoordinate.xy + 4.0 * singleStepOffset);
    textureShift_3 = vec4(textureCoordinate.xy - 6.0 * singleStepOffset, textureCoordinate.xy + 6.0 * singleStepOffset);
    textureShift_4 = vec4(textureCoordinate.xy - 8.0 * singleStepOffset, textureCoordinate.xy + 8.0 * singleStepOffset);
}