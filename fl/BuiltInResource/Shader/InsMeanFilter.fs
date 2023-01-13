#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#else
#define highp
#define mediump
#define lowp
#endif

uniform sampler2D inputImageTexture;
varying  vec2 textureCoordinate;
uniform  float texelWidthOffset;
uniform  float texelHeightOffset;
void main()
{
    mediump vec4 oriColor = texture2D(inputImageTexture, textureCoordinate);
    mediump vec4 sum = oriColor;
    mediump vec4 Color = texture2D(inputImageTexture, textureCoordinate+vec2(-texelWidthOffset,-texelHeightOffset))*0.5;
    sum += Color;
    Color = texture2D(inputImageTexture, textureCoordinate+vec2(texelWidthOffset,texelHeightOffset))*0.5;
    sum += Color;
    Color = texture2D(inputImageTexture, textureCoordinate+vec2(-2.0*texelWidthOffset,-2.0*texelHeightOffset))*0.2;
    sum += Color;
    Color = texture2D(inputImageTexture, textureCoordinate+vec2(2.0*texelWidthOffset,2.0*texelHeightOffset))*0.2;
    sum += Color;
    sum  = sum / 2.4;
    gl_FragColor = mix(oriColor, sum, 1.0 - oriColor.a);
}