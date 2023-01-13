attribute vec2 position;
uniform  float texelWidthOffset;
uniform  float texelHeightOffset;

varying vec2 textureCoordinate;
varying vec4 textureShift_1;
varying vec4 textureShift_2;
varying vec4 textureShift_3;
varying vec4 textureShift_4;

void main()
{
    gl_Position = vec4(position*2.0-1.0,0.5,1.0);

    vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
    textureCoordinate = position;
    textureShift_1 = vec4(position.xy - 2.0 * singleStepOffset, position.xy + 2.0 * singleStepOffset);
    textureShift_2 = vec4(position.xy - 4.0 * singleStepOffset, position.xy + 4.0 * singleStepOffset);
    textureShift_3 = vec4(position.xy - 6.0 * singleStepOffset, position.xy + 6.0 * singleStepOffset);
    textureShift_4 = vec4(position.xy - 8.0 * singleStepOffset, position.xy + 8.0 * singleStepOffset);
}